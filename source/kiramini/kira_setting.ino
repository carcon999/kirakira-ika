/*
 * kira_setting.ino
 * Web設定処理
 * 
 * 設定モードの選択とWebサーバの起動
 * 
 */
#include <Arduino.h>
#include "def.h"
#include "drv_btn.h"
#include "drv_axis3.h"
#include "drv_barled.h"
#include "config.h"
#include "comlib.h"
#include "webserver.h"

using namespace kira;

enum eSettingMode{
  SETMODE_SELECT,
  SETMODE_WEB
};

typedef enum eSettingMode ModeSet_t;

static ModeSet_t mode_set = SETMODE_SELECT;

// 設定処理ループ
void loop_kr_setting(void)
{
  if(mode_set == SETMODE_SELECT){
    kr_setting_select();
  }else if(mode_set == SETMODE_WEB){
    kr_setting_web();
  }
  
  BTN.loop();
}

// 設定モード開始
void kr_setting_start(void)
{
  uint8_t rgb[LED_RGB_SIZE] = {5, 5, 5};  // 白
  LED.set_color(rgb);         // 白

  // 設定モードへ移行
  gAppMode = APPMODE_SET;
}

// 設定選択
void kr_setting_select(void)
{
  static uint8_t old_dir;
  uint8_t now_dir = AX3.get_tilt();

  //前回値と方向が異なる場合
  if(now_dir != old_dir)
  {
    // 下方向
    if(now_dir == TILT_DOWN){
      uint8_t rgb[LED_RGB_SIZE] = {5, 5, 5};  // 白
      LED.set_color(rgb);         // 白
    // 左方向  
    }else if(now_dir == TILT_LEFT){
      LED.set_color(0, 255, 5);   // 赤
    // 上方向
    }else if(now_dir == TILT_UP){
      LED.set_color(120, 255, 5); // 青
    // 右方向
    }else if(now_dir == TILT_RIGHT){
      LED.set_color(240, 255, 5); // 緑
    }
    
    old_dir = now_dir;
  }

  eButtonStatus sts = BTN.get();
    
  // ボタン押下により設定処理の実行
  if(sts == Pushed){
    BTN.clear();

    if(now_dir == TILT_DOWN){
      // 下方向

    }else if(now_dir == TILT_LEFT){
      uint8_t grb[LED_RGB_SIZE] = {0, 20, 0};  // 
      // 左方向
      LED.turn_off();
      delay(100);
      LED.set_color(0, grb);
      LED.update();
      delay(100);

      // 停止しないとWebServerが動けない場合があるので、必ずLEDは停止
      LED.end();
      // 設定ブラウザセットアップ
      WEB.setup();

      mode_set = SETMODE_WEB;

    }else if(now_dir == TILT_UP){
      // 上方向
      uint8_t grb[LED_RGB_SIZE] = {20, 0, 0};  // 
      LED.turn_off();
      delay(100);
      LED.set_color(0, grb);
      LED.update();
      delay(100);

      // WebSocketを有効に設定変更し通常モードへ
      CONF.wifi = 1;
      CONF.save();

      startup_normal_mode();

    }else if(now_dir == TILT_RIGHT){
      // 右方向
      uint8_t grb[LED_RGB_SIZE] = {0, 0, 20};  // 
      LED.turn_off();
      delay(100);
      LED.set_color(0, grb);
      LED.update();
      delay(100);

      // WebSocketを有効に設定変更し再起動する。
      CONF.wifi = 0;
      CONF.save();

      startup_normal_mode();
    }
  }

  delay(100);
}

void kr_setting_web(void)
{
  eButtonStatus sts = BTN.get();

  if(sts == Pushed){
    // 強制的にリセット
    ComLib::reset();
  }

  WEB.loop();
}


