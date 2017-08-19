/*
 * kiramini.ino
 * きらきら☆いかメイン処理
 * 
 * スタートアップ処理＋メインループ処理の記述
 * 
 */
#include <Arduino.h>
#include "def.h"
#include "image.h"
#include "drv_btn.h"
#include "drv_axis3.h"
#include "drv_barled.h"
#include "config.h"
#include "comlib.h"
#include "drv_wss.h"

using namespace kira;

AppMode_t gAppMode = APPMODE_SET;
uint32_t  gTimeout;

void setup()
{
  pinMode(ESP_PIN, OUTPUT);
  digitalWrite(ESP_PIN, HIGH);

  // チップLED点灯
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  // ボタン
  BTN.setup();

  USE_SERIAL.begin(115200);
  USE_SERIAL.println("\nstart");
  delay(100);

  // 設定ファイルロード
  CONF.load();

  // 3軸加速度センサー
  AX3.setup();
  // 下向きで起動した場合は設定モード
  if(AX3.get_tilt() == TILT_DOWN)
  {
    // 設定モードへ
    startup_setting_mode();

  // 通常起動モード
  }else{
    // 通常モードへ
    startup_normal_mode();
  }
}

// 設定モードへの移行初期化
void startup_setting_mode(void)
{
  USE_SERIAL.println("Setting Mode.");

  LED.end();

  // Wi-Fiリモコン 停止設定
  ws_remo_enabled(false);
    
  // フルカラーLED初期化
  LED.begin(CONF.length);

  // 設定モード
  kr_setting_start();
}

// 通常モードへの移行初期化
void startup_normal_mode(void)
{
  USE_SERIAL.println("Normal Mode.");

  // Wi-Fi設定中はLEDバーは停止する。
  LED.end();

  // Wi-Fiセットアップする。
  ws_remo_enabled(CONF.wifi != 0);
  
  // フルカラーLED初期化
  LED.begin(CONF.length);

  // 更新が必要な場合は、ファイルの生成処理を実施する。
  if(CONF.update){

    USE_SERIAL.println("export start");
    IMG.export_use_file(CONF.bright, true, export_callback);
    IMG.create_string_file(CONF.bright);
    USE_SERIAL.println("export end");
      
    // フラグクリアし設定ファイル保存
    CONF.update = 0;
    CONF.save();
  }

  // 単色表示モード
  kr_color_start(false);
}

// 進捗表示
static void export_callback(void)
{
  static uint8_t counter = 0;
  LED.set_progress(CONF.bright, counter++);
}

const FUNCPTR tMode[APPMODE_NUM] = {
  loop_kr_setting,
  loop_kr_color,
  loop_string,
  loop_image
};

void loop()
{
  (*tMode[gAppMode])();

  // Wi-Fiリモコン処理
  ws_remo_loop();
}

