/*
 * kira_string.ino
 * 文字メッセージ表示処理
 * 
 * 1文字ずつの送り出し表示処理
 * 
 */
#include <Arduino.h>
#include "def.h"
#include <FS.h>
#include "image.h"
#include "drv_wss.h"
#include "drv_btn.h"
#include "drv_axis3.h"
#include "drv_barled.h"
#include "webserver.h"
#include "config.h"
#include "comlib.h"
#include "webcommon.h"
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson

using namespace kira;

static long fs_size;
static File fsw;

extern void func_operation_check(void);

enum eModeString{
  STR_WAIT_LEFT,    // 文字左振り開始待ち
  STR_PROC_LEFT,    // 文字左振り表示処理
  STR_WAIT_RIGHT,   // 文字右振り開始待ち
  STR_PROC_RIGHT,   // 文字右振り表示処理
  STR_WRITE_FIN,    // 書込み完了待ち
  STR_MODE_NUM
};
typedef enum eModeString ModeStr_t;

// 動作モード
static ModeStr_t mode_str= STR_WAIT_LEFT;

typedef void (*FUNCPTR)(void);

const FUNCPTR tStrFunc[STR_MODE_NUM][4] = {
  {kr_str_key,      kr_str_wait_left,     kr_str_nop,         kr_str_nop},        // 
  {kr_str_nop,      kr_str_nop,           kr_str_nop,         kr_str_proc_left }, // 
  {kr_str_key,      kr_str_nop,           kr_str_wait_right,  kr_str_nop},        // 
  {kr_str_nop,      kr_str_nop,           kr_str_nop,         kr_str_proc_right}, // 
  {kr_str_wait,     kr_str_wait,          kr_str_wait,        kr_str_wait},       // 書込み完了待ち
};

void loop_string(void)
{
  int fac = NON;
  
  // タイムアウトチェック
  if(micros() > gTimeout){
    gTimeout = micros() + LED_DELAY;
    fac = 3;    // タイムアウト
    
  }else{
    fac = AX3.shake_dir();
  }

  // function call
  (*tStrFunc[mode_str][fac])();

  BTN.loop();
}

void kr_str_start(void)
{
  USE_SERIAL.printf("func_str_start\n");

  // 表示は消灯する。
  LED.turn_off();

  // 文字ファイルアクセス初期化
  if(IMG.setup(STRING)){
    // 文字表示モードへ移行
    gAppMode = APPMODE_STR;
    mode_str= STR_WAIT_LEFT;
    gTimeout = micros() + LED_DELAY;
  
  }else{
    // 文字表示できないので画像へ
    kr_img_start();
  }
}

// WebSocketのバイナリ受信イベント処理
void WSS_OnEvent_Bin(uint8_t num, uint8_t * payload, size_t length)
{
  if(fs_size > 0){
    // ファイル書き込み
    fsw.write(payload, length);
    fs_size -= length;

    USE_SERIAL.printf("fs_size : %d, len : %d \n", fs_size, length);
    // 最後の場合は、Closeする
    if(fs_size <= 0){
      fsw.close();
    }
  }
}

// 文字列データ書き込み開始
static void kr_str_write_start(String name, long size)
{
  //String filename = IMG.get_work_folder_name(STRING);
  String filename ="/str/" + name;
  fs_size = size;

  // 既存のファイルアクセスは終了
  IMG.close();

  // ファイルが存在したら削除する。
  if(SPIFFS.exists(filename)){
    SPIFFS.remove(filename);
  }

  // ファイル書き込みの準備
  fsw = SPIFFS.open(filename, "w");

  USE_SERIAL.printf("%s, size %d\n", filename.c_str() , fs_size);

  // 文字表示モードへ移行
  gAppMode = APPMODE_STR;
  gTimeout = micros() + LED_DELAY;
  // 書込み完了まで待つ
  mode_str = STR_WRITE_FIN;  
}

// 書込み完了待ち
static void kr_str_wait(void)
{
  // 全て完了したら文字列用ビットマップを作成する。
  if(fs_size <= 0){
    USE_SERIAL.printf("str finished.\n");
    // ビットマップ文字列の生成
    IMG.create_string_file(CONF.bright);
    USE_SERIAL.printf("created string bmp files.\n");
    // 文字列表示開始
    kr_str_start();
  }
}

static bool kr_str_drawline(int dir)
{
  int ret = false;
  uint8_t buff[LED_MAX_LENGTH][LED_RGB_SIZE];

  ret = IMG.get_next_line(dir, buff);  
  if(ret)
  {
    LED.set_color(buff);
  }

  return ret;
}

static void kr_str_key(void)
{
  eButtonStatus sts = BTN.get();

  if(sts == Pushed){
    BTN.clear();
    // 画像表示へ
    kr_img_start();
  }else if(sts == LongPushed){
    BTN.clear();
  }

  // Wifiからの操作
  func_operation_check();
}

static void kr_str_wait_left(void)
{
  mode_str = STR_PROC_LEFT;
}

static void kr_str_proc_left(void)
{
  if(!kr_str_drawline(1)){
    mode_str = STR_WAIT_RIGHT;
  }
}

static void kr_str_wait_right(void)
{
  mode_str = STR_PROC_RIGHT;
}

static void kr_str_proc_right(void)
{
  if(!kr_str_drawline(-1)){
    mode_str = STR_WAIT_LEFT;
  }  
}

static void kr_str_nop(void)
{
}

