
#include "arduino.h"
#include "image.h"
#include "config.h"
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson
#include "comlib.h"

using namespace kira;

#define DEF_BMP_WIDTH_MAX   256

// global instance 
kira::Image IMG;

// フォルダ場所
const char* tFolder[WORK_FSIZE] = {
  "/tmp","/tmp_str"
};

static int pos_count = 0;
static int posx = 0;
static int pos_idx = 0;
static int pos_idx_max = 0;

// 初期化処理を行う関数
// コンストラクタは　クラス名::クラス名と同じ名前で構成します
Image::Image()
{
  clear();
}

bool Image::setup(eFolder_t f)
{
  bool ret = false;
  
  SPIFFS.begin();

  ft = f;

  finf.index = 0;
  finf.file_count = 0;

  // ファイル数を算出する。
  Dir dir;
  String s = get_work_folder_name(ft);
  s += "/";
  dir = SPIFFS.openDir(s);

  while (dir.next()) {
    //USE_SERIAL.println(dir.fileName());
    finf.file_count++;
  }

  // ファイル見つからず・・・
  if(finf.file_count == 0){
    goto EXIT;
  }

  // 先頭画像ロード
  load();

  ret = true;

EXIT:;

  return ret;
}

void Image::close(void)
{
  clear();
}

void Image::clear(void)
{
  if(file != null){
    file.close();
  }
  
  memset(&inf, 0x00, sizeof(inf));
  memset(&finf, 0x00, sizeof(finf));

  pos_count = 0;
  posx = 0;
  pos_idx = 0;
  pos_idx_max = 0;
}

bool Image::load(void)
{
  bool ret = false;
  char file_name[20];

  // ファイル名を生成する。
  sprintf(file_name, "%s/%d.bmp", get_work_folder_name(ft), finf.index);

  USE_SERIAL.printf("Load image : %s\n", file_name);
  if(!open_bitmap_file(file_name)){
    goto EXIT;
  }

  if(ft == IMAGE){
    // 最大画像幅単位で区切る
    set_range(DEF_BMP_WIDTH_MAX);
  }else{
    // 1文字単位で区切る
    set_range(CONF.length);    
  }
  ret = true;
  
EXIT:;

  return ret;
}

bool Image::load_top(void)
{
  finf.index = 0;
  load();
  return true;  
}

bool Image::load_next(void)
{
  bool ret = false;
  finf.index++;

  if(finf.index >= finf.file_count){
    finf.index = 0;
    ret = false;
    goto EXIT;
  }

  // Image Load
  load();
  ret = true;

EXIT:;

  return ret;
}

bool Image::load_before(void)
{
  bool ret = false;
  finf.index--;

  if(finf.index < 0){
    finf.index = finf.file_count - 1;
    ret = false;
    goto EXIT;
  }

  // Image Load
  load();
  ret = true;

EXIT:;

  return ret;  
}

void Image::clear_folder(const char* fname)
{
  // tmpフォルダ内のファイル全削除
  Dir dir = SPIFFS.openDir(fname);
  while (dir.next()) {
    SPIFFS.remove(dir.fileName());
  }  
}

///////////////////////////////////////////////////////////////////////////////
// @brief  ビットマップファイルのオープン処理
// @param[in] name :ファイル名
// @return TRUE:正常　FALSE:エラー
// @detail 
///////////////////////////////////////////////////////////////////////////////
bool Image::open_bitmap_file(char* name)
{
  uint8_t buff[128];
  uint8_t ret = false;
  uint32_t* ptmp32;
  uint16_t* ptmp16;
  uint8_t* phead = &buff[0];

  memset((void*)&inf, 0x00, sizeof(inf)); 

  ///////////////////////////////////////////////////////////////////////////
  // ファイルOPEN
  ///////////////////////////////////////////////////////////////////////////
  file = SPIFFS.open(name, "r");
  if(file == null){
    USE_SERIAL.println("open error!");
    goto EXIT;    
  }

  // 先頭14byteを読み込む
  file.seek(0, SeekSet);
  if(14 != file.read(buff, 14)){
    USE_SERIAL.println("read error!");
    goto EXIT;
  }
  
  ///////////////////////////////////////////////////////////////////////////
  // BMチェック
  ///////////////////////////////////////////////////////////////////////////
  if(*phead != 0x42 || *(phead+1) != 0x4D){
    USE_SERIAL.println("BM error!");
    goto EXIT;
  }

  ///////////////////////////////////////////////////////////////////////////
  // 画像データチェック
  ///////////////////////////////////////////////////////////////////////////
  // 情報ヘッダまで更に読み込む
  if(40 != file.read(&buff[14], 40)){
    USE_SERIAL.println("40 byte error!");
    goto EXIT;
  }

  // ファイル名のコピー
  strcpy(&inf.filename[0], name);
  
  // 画像のサイズチェック（幅）
  ptmp32 = (uint32_t*)(phead + 18);
  inf.width = (uint16_t)*ptmp32;

  // 画像のサイズチェック（高さ）
  ptmp32 = (uint32_t*)(phead + 22);
  inf.height = (uint16_t)*ptmp32;

  // ビットカウントは24bitのみ
  inf.bitcount = *(uint16_t*)(phead + 28);
  
  // 画像へのオフセット保持
  inf.offset = *(uint32_t*)(phead + 10);
  
  // 画像のパディング計算
  {
    uint16_t line = (inf.width * LED_RGB_SIZE);
    uint8_t tmp = (uint8_t)(line % 4);
    if ( tmp != 0 ){
      tmp = 4 - tmp;
    }
    inf.padding = tmp;
    inf.linesize = line + inf.padding; 
  }

  ret = true;

EXIT:;

  if(!ret){
    if(file != null){
      file.close();
    }
  }

  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// @brief  作業用の画像ファイルを作成する。
// @return なし
// @detail Open中のファイルを90度回転して作業用フォルダにコピーする。
///////////////////////////////////////////////////////////////////////////////
bool Image::create_rotation_file(uint8_t bright, int index)
{
  const uint8_t padding_data[] = {0xFF, 0xFF, 0xFF, 0xFF};
  uint8_t rgb[LED_RGB_SIZE];
  uint8_t gbr[LED_RGB_SIZE];
  uint8_t buff[256];
  uint32_t* ptmp32;
  int padding;
  bool ret = false;
  float br = (float)(255-bright)/255;

  // tmpファイル以下に番号で生成する。
  sprintf((char*)buff, "%s/%d.bmp", tFolder[IMAGE], index);

  USE_SERIAL.println((char*)buff);

  File f = SPIFFS.open((char*)buff, "w");
  if (!f) {
    USE_SERIAL.println("file open failed");
    goto EXIT;
  }

  if(inf.offset > sizeof(buff)){
    USE_SERIAL.println("overflow bmp header");
    goto EXIT;    
  }

  // BMPヘッダーを読み込む
  file.seek(0, SeekSet);
  if(inf.offset != file.read(buff, inf.offset)){
    USE_SERIAL.println("bmp header read error.");
    goto EXIT;
  }

  // 画像のサイズ（幅）を変更（入れ替え）
  ptmp32 = (uint32_t*)&buff[18];
  *ptmp32 = inf.height;

  // 画像のサイズ（高さ）を変更
  ptmp32 = (uint32_t*)&buff[22];
  *ptmp32 = inf.width;

  if(inf.offset != f.write(buff, inf.offset)){
    USE_SERIAL.println("bmp header write error.");
    goto EXIT;
  }

  // 画像のパディング計算
  padding = (inf.height * LED_RGB_SIZE) % 4;
  if ( padding != 0 ){
    padding = 4 - padding;
  }

  
  for(int w = 0 ; w < inf.width ; w++)
  {
    for(int h = 0 ; h < inf.height ; h++)
    {
      file.seek(inf.offset + h*inf.linesize + (w * LED_RGB_SIZE), SeekSet);
      if(LED_RGB_SIZE == file.read(rgb, LED_RGB_SIZE)){

        // 明るさの変換処理
        convert_brightness(rgb, br);
        
        gbr[0] = rgb[1]; // R <- G
        gbr[1] = rgb[2]; // G <- B
        gbr[2] = rgb[0]; // B <- R
        
        f.write(gbr, LED_RGB_SIZE);
      }
    }

    // パディングが必要な場合は書き込む
    if(padding){
      f.write(padding_data, padding);
    }
  }

  f.close();
  ret = true;

EXIT:;

  if(ret){
    USE_SERIAL.println("success.");
  }else{
    USE_SERIAL.println("failed.");
  }

  return ret;
}

#define DEF_SEP_MAX     (8)
#define DEF_SEP_REP     (1)

int tMaxRange[DEF_SEP_MAX][2];

// 文字列データの１文字範囲設定
void Image::set_range(int len)
{
  int unit = len;

  for(int i = 0 ; i < DEF_SEP_MAX ; i++){
    if(inf.height > unit){
      tMaxRange[i][0] = unit - len;
      tMaxRange[i][1] = unit - 1;
      unit += len;     
    }else{
      tMaxRange[i][0] = unit - len;
      tMaxRange[i][1] = inf.height - 1;
      pos_idx_max = i + 1;
      break;
    }
  }

  pos_count = 0;
  posx = -1;
  pos_idx = 0;
}

bool Image::get_next_line(int dir, uint8_t buff[LED_MAX_LENGTH][LED_RGB_SIZE])
{
  static bool is_end = false;
  int ret = false;
  int line_pos;

  // 最終判定
  if(is_end){
    is_end = false;
    goto EXIT;
  }

  // 方向の加算
  posx += dir;

  // 範囲外は終了
  if(posx < tMaxRange[pos_idx][0]){
    pos_count++;

//  USE_SERIAL.println(pos_count);

    // 指定回以上繰り返したら・・・
    if(pos_count >= DEF_SEP_REP){
      pos_count = 0;
      pos_idx++;
      // 最大を超える場合は次にする。
      if(pos_idx >= pos_idx_max){
        pos_idx = 0;

        // 文字表示の場合は、ファイルも変更する。
        if(ft == STRING){
          if(!load_next()){
            load_top();
          }
        }
      }
    }
    posx = tMaxRange[pos_idx][0] - 1;
    
    goto LAST;
  }

  if(posx > tMaxRange[pos_idx][1]){
    posx = tMaxRange[pos_idx][1] + 1;
    goto LAST;
  }

  // データを作成する
  line_pos = inf.offset + posx * inf.linesize;
  file.seek(line_pos, SeekSet);
  file.read(&buff[0][0], inf.width*LED_RGB_SIZE);
  ret = true;

  goto EXIT;

LAST:;
  is_end = true;
  memset(&buff[0][0],0x00, inf.width*LED_RGB_SIZE);
  ret = true;

EXIT:;
  return ret;
}

bool Image::get_repeat_line(uint8_t buff[LED_MAX_LENGTH][LED_RGB_SIZE])
{
  int ret = true;

  // 方向の加算
  posx++;

  if(posx >= inf.height){
    posx = 0;
    ret = false;
  }

  // データを作成する
  int line_pos = inf.offset + posx * inf.linesize;
  file.seek(line_pos, SeekSet);
  file.read(&buff[0][0], inf.width*LED_RGB_SIZE);

EXIT:;
  return ret;
  
}

#define FILE_MAX      (100)
#define FILE_NAME_SZ  (20)
void Image::export_use_file(uint8_t bright, bool bForce, CALLBACK pFunc)
{
  int count = 0;
  char* pTopfilelist = (char*)malloc(FILE_MAX * FILE_NAME_SZ);
  char* pName = pTopfilelist;
  char tmp[20];

  // コールバック呼び出し（進捗ステータス）
  (*pFunc)();
  
  // 強制処理の場合は、tmpフォルダ以下のファイルを全削除
  if(bForce){
    clear_folder(tFolder[IMAGE]);
  }

  // コールバック呼び出し（進捗ステータス）
  (*pFunc)();

  // ファイル名の一覧を作成する。
  // Dirを利用している最中にファイル追加するとズレる
  Dir dir = SPIFFS.openDir(ComLib::get_master_img_folder());
  USE_SERIAL.printf("FileNames: %d", dir.fileSize());

  String fname;
  while (dir.next()) {
    fname = dir.fileName();
    fname.toLowerCase();
    // 一応拡張子.bmpだけにする。
    if(fname.endsWith(".bmp")){
      sprintf(pName, "%s", dir.fileName().c_str());
      USE_SERIAL.println(pName);
      count++;
      if(count >= FILE_MAX){
        break;
      }
      pName += FILE_NAME_SZ;
    }
  }
  USE_SERIAL.printf("count:%d", count);

  // コールバック呼び出し（進捗ステータス）
  (*pFunc)();

  // ソート処理する。
  for(int i = 1 ; i < count; i++){
    for(int j = 1 ; j < count; j++){
      if(strcmp(pTopfilelist + FILE_NAME_SZ * (j-1), pTopfilelist + FILE_NAME_SZ * j) > 0){
        strcpy(tmp, pTopfilelist + FILE_NAME_SZ * (j-1));
        strcpy(pTopfilelist + FILE_NAME_SZ * (j-1), pTopfilelist + FILE_NAME_SZ * j);
        strcpy(pTopfilelist + FILE_NAME_SZ * j, tmp);
      }
    }
  }

  // コールバック呼び出し（進捗ステータス）
  (*pFunc)();

  pName = pTopfilelist;
  // orgフォルダ内のファイルを調べてファイルを作成する
  for(int i = 0 ; i < count ; i++){
    // OPEN成功したファイルは作業エリアへExportする。
    USE_SERIAL.print("export file :");
    USE_SERIAL.println(pName);

    // コールバック呼び出し（進捗ステータス）
    (*pFunc)();

    if(open_bitmap_file(pName)){
      create_rotation_file(bright, i);
      // コールバック呼び出し（進捗ステータス）
      (*pFunc)();
    }
    pName += FILE_NAME_SZ;
  }

  free(pTopfilelist);
}

// 文字表示用ファイルを生成する。
void Image::create_string_file(uint8_t bright)
{
  uint8_t ret = false;

  ///////////////////////////////////////////////////////////////////////////
  // 文字列設定ファイルOPEN
  ///////////////////////////////////////////////////////////////////////////
  if (SPIFFS.begin()) {

    // JSONファイル有無判定
    if (SPIFFS.exists("/str/str.json")) {

      //file exists, reading and loading
      USE_SERIAL.println("reading config file");
      File f = SPIFFS.open("/str/str.json", "r");
      if (f) {
        USE_SERIAL.println("opened config file");
        size_t size = f.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        f.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) 
        {
          USE_SERIAL.println("\nparsed json");
          String tp = json["type"];

          // 文字列バイナリ指定
          if(tp == "str"){

            // 古いデータは削除する。
            clear_folder(get_work_folder_name(STRING));
      
             // データの取り出しは暗黙の型変換
            String fname = json["name"];    // 文字列バイナリのファイル名
            int led_sz = json["leds"];      // LED数
            int led_index = json["pallet"]; // 色Index

            String s1 =  "/str/" + fname;
            USE_SERIAL.printf("name: %s\n", s1.c_str());
      
            File fbin = SPIFFS.open(s1 , "r");

            int index = 0;
            int fbin_size = fbin.size();
            while(fbin_size > 0){
              int sz = led_sz * 8 * 4;  // 最大読み込みサイズ
              if(fbin_size < sz){
                sz = fbin_size;
              }

              USE_SERIAL.printf("create_string_bitmap:%d, %d", index, sz);

              create_string_bitmap(fbin, bright, led_index, index++, sz, led_sz);
              fbin_size -= sz;   
            }

            fbin.close();    
          }
          
        } else {
          USE_SERIAL.println("failed to load json config");
        }
      }

    // Jsonファイルなし
    }else{
      USE_SERIAL.println("nothig string file");
      goto EXIT;
    }
  }

EXIT:;
}

void Image::create_string_bitmap(File f, uint8_t br, uint8_t led_index, int fno, int sz, int led_sz){
  uint8_t tbmpHeader[] = {
    0x42, 0x4D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 
    0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };

  File fbmp;

  // 全体のデータサイズ
  int w = 32;
  int h = (sz >> 2);
  int hsize = sizeof(tbmpHeader);
  int total = hsize + (w * h * 3);
  
  // ファイルサイズ
  uint32_t* psize = (uint32_t*)&tbmpHeader[2];
  *psize = total;
  
  // 画像の幅
  uint32_t* pwidth = (uint32_t*)&tbmpHeader[18];
  *pwidth = w;

  // 画像の高さ
  uint32_t* pheight = (uint32_t*)&tbmpHeader[22];
  *pheight = h;

  // ファイル名
  char buff[20];
  sprintf(buff, "/tmp_str/%d.bmp", fno);

  USE_SERIAL.printf("create file %s\n", buff);

  // ファイルを生成する。
  fbmp = SPIFFS.open((char*)buff, "w");
  if (!fbmp) {
    USE_SERIAL.println("file open failed");
    goto EXIT;
  }

  // BMPヘッダーを書き込む
  if (hsize != fbmp.write(tbmpHeader, hsize)){
    USE_SERIAL.println("bmp header write failed");
    goto EXIT;    
  }

  // データ部の生成
  {
    int wsize = led_sz * 32 * LED_RGB_SIZE;
    uint8_t* pbuffTop =(uint8_t*)malloc(wsize);
    uint8_t* pbuffTmp = pbuffTop;
    uint8_t grb[LED_RGB_SIZE];
    ComLib::get_index_to_grb(br, led_index, grb);

//  USE_SERIAL.printf("h = %d\n", h);

    // ビットデータを順番に32bit単位で読み込み処理する。
    for (int y = 0; y < (h / led_sz); y++) {      

//    USE_SERIAL.printf("loop = %d\n", y);
      // 黒でクリア
      memset(pbuffTop, 0x00, wsize);
      pbuffTmp = pbuffTop;

      // 1文字分ごとのサイズで書き込み（高速化）
      uint32_t bitdata[32];
      if(f.read((uint8_t*)&bitdata[0], 4*led_sz) == 4*led_sz){
        for (int n = 0 ; n < led_sz ; n++){
          uint32_t tmp = bitdata[n];
          for (int x = 0 ; x < w ; x++) {
            if(tmp & 0x01){
              *pbuffTmp = grb[0]; pbuffTmp++;
              *pbuffTmp = grb[1]; pbuffTmp++;
              *pbuffTmp = grb[2]; pbuffTmp++;
            }else{
              pbuffTmp += 3;
            }
            tmp >>= 1;
          }
        }

        if(wsize != fbmp.write(pbuffTop, wsize)){
          USE_SERIAL.println("bmp data write failed.");
          goto EXIT;
        }
      }
    }

    free(pbuffTop);
  }
EXIT:;

  if(fbmp){
    fbmp.close();
  }
}

const char* Image::get_work_folder_name(eFolder_t f)
{
  return tFolder[f];
}
    
// 画像の明るさ変換
void Image::convert_brightness(uint8_t rgb[LED_RGB_SIZE], float offset)
{
  // オフセット無しは処理しない。
  if(offset == 0){
    return;
  }

  // 黒は変換しない
  int sum = rgb[0] + rgb[1] + rgb[2];
  if(sum == 0){
    return;
  }

  // RGBをHSV変換
  {
    char buff[80];
    float H, S, V;
    rgb_to_hsv(rgb, &H, &S, &V);
  
    // HSVのBrightnessを変更
    V -= offset;
    if(V < 0){
      // 0だと黒になるので多少の色がでるようにする。
      V = 0.01;
    }

    // HSVをRGB変換
    hsv_to_rgb(H, S, V, rgb);
  }
}

void Image::rgb_to_hsv(uint8_t rgb[LED_RGB_SIZE], float* H, float* S, float* V)
{
  float max;
  float min;
  float val;
  float R, G, B;

  // 0.0 - 1.0（255分率）
  R = (float)rgb[0] / 255;
  G = (float)rgb[1] / 255;
  B = (float)rgb[2] / 255;

  // 最大値・最小値
  if (R >= G && R >= B) {
    max = R;
    min = (G < B) ? G : B;
  } else if (G >= R && G >= B) {
    max = G;
    min = (R < B) ? R : B;
  } else {
    max = B;
    min = (R < G) ? R : G;
  }

  val = max - min;
  
  if(val == 0){
    *H = 0;
  }else{
    // Hue（色相）
    if (max == R) {
      *H = 60 * (G - B) / val;
    } else if (max == G){
      *H = 60 * (B - R) / val + 120;
    } else {
      *H = 60 * (R - G) / val + 240;
    }
    if (*H < 0.0) {
      *H += 360.0;
    }
  }

  // Saturation（彩度）
  if(max != 0){
    *S = val / max;
  }else{
    *S = 0;
  }
  // Value（明度）
  *V = max;
}

void Image::hsv_to_rgb(float H, float S, float V, uint8_t rgb[LED_RGB_SIZE])
{
  int Hi;
  float f, p, q, t;

  Hi = ((int)(H / 60)) % 6;
  f = H / 60 - Hi;
  p = V * (1 - S);
  q = V * (1 - f * S);
  t = V * (1 - (1 - f) * S);

  // 256階調に戻す
  V *= 255;
  p *= 255;
  q *= 255;
  t *= 255;

  switch (Hi) {
    case 0: rgb[0] = (uint8_t)V; rgb[1] = (uint8_t)t; rgb[2] = (uint8_t)p; break;
    case 1: rgb[0] = (uint8_t)q; rgb[1] = (uint8_t)V; rgb[2] = (uint8_t)p; break;
    case 2: rgb[0] = (uint8_t)p; rgb[1] = (uint8_t)V; rgb[2] = (uint8_t)t; break;
    case 3: rgb[0] = (uint8_t)p; rgb[1] = (uint8_t)q; rgb[2] = (uint8_t)V; break;
    case 4: rgb[0] = (uint8_t)t; rgb[1] = (uint8_t)p; rgb[2] = (uint8_t)V; break;
    case 5: rgb[0] = (uint8_t)V; rgb[1] = (uint8_t)p; rgb[2] = (uint8_t)q; break;
  }
}

