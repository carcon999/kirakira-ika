#include "arduino.h"
#include "comlib.h"
#include <FS.h>

using namespace kira;

// 点滅Delay
void ComLib::delay_and_led(int ms, int blink)
{
  USE_SERIAL.printf("WAIT %d...\n", ms);
  USE_SERIAL.flush();

  if(blink != 0)
  {
    int count = ms / blink;

    digitalWrite(LED_PIN, HIGH);
    for(int i = 0 ; i < count ; i++){
      delay(blink);
      USE_SERIAL.print(".");
      digitalWrite(LED_PIN, i & 0x01);
    }
    digitalWrite(LED_PIN, HIGH);

  }else{
    delay(ms);
  }
  USE_SERIAL.println("\nWAIT End");
  USE_SERIAL.flush();
}

// ESPリセット
void ComLib::reset(void)
{
  USE_SERIAL.print("ESP Self Reset");
  USE_SERIAL.print(".");
  USE_SERIAL.flush();
  delay(100);
  USE_SERIAL.print(".");
  USE_SERIAL.flush();
  delay(100);
  USE_SERIAL.print(".");
  USE_SERIAL.flush();
  delay(100);

  // 高速点滅
  delay_and_led(1000, 100);

  SPIFFS.end();
  wdt_disable();
  ESP.reset();
}

// CSV形式の変換処理
int ComLib::splitCsv(String data, String (&dst)[CSV_MAX]){
  int index = 0;
  int datalength = data.length();

  for (int i = 0; i < datalength; i++) {
    char tmp = data.charAt(i);
    if ( tmp == ',' ) {
      index++;
      if ( index > (CSV_MAX - 1)) return -1;
    }
    else dst[index] += tmp;
  }

  USE_SERIAL.println("splitcsv len: " + String(index + 1));

  return (index + 1);
}

//format bytes
String ComLib::formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

// ビットマップ用の色を生成
void ComLib::get_index_to_grb(uint8_t br, uint8_t index, uint8_t grb[LED_RGB_SIZE]){
  if(index == 0){       // 白
    grb[0] = br; grb[1] = br; grb[2] = br;
  }else if(index == 1){ // 青
    grb[0] = 0; grb[1] = 0; grb[2] = br;
  }else if(index == 2){ // 緑
    grb[0] = br; grb[1] = 0; grb[2] = 0;
  }else if(index == 3){ // 水
    grb[0] = br; grb[1] = 0; grb[2] = br;
  }else if(index == 4){ // 橙
    grb[0] = br/2; grb[1] = br; grb[2] = 0;
  }else if(index == 5){ // 赤
    grb[0] = 0; grb[1] = br; grb[2] = 0;
  }else{                // 白
    grb[0] = br; grb[1] = br; grb[2] = br;
  }
}

// マスター画像フォルダの取得
const char* ComLib::get_master_img_folder(void)
{
  return "/img/";
}


