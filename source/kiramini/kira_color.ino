/*
 * kira_color.ino
 * 単色表示
 * 
 * 単色表示処理
 * 
 */
#include <Arduino.h>
#include "def.h"
#include "drv_barled.h"
#include "config.h"

using namespace kira;

static unsigned long update_led_tout = 0;
static bool update_led_color_flag = false;

extern void func_operation_check(void);

// 単色表示ループ
void loop_kr_color(void)
{
  kr_color_main();

  BTN.loop();
}

// 色表示モード開始
void kr_color_start(bool hispd)
{
  uint8_t grb[LED_RGB_SIZE] = {CONF.rgb[1], CONF.rgb[0], CONF.rgb[2]};

  // 単色表示モードへ移行
  gAppMode = APPMODE_COLOR;

  if(hispd){
    LED.set_color(grb);    
  }else{
    LED.turn_on(grb);
  }

  // 開始後１秒
  update_led_tout = millis() + 1000;
}

// 色表示モード
void kr_color_main(void)
{
  eButtonStatus sts = BTN.get();

  // ボタン押下により画像表示へ
  if(sts == Pushed){
    BTN.clear();

    // 画像表示へ
    kr_str_start();
  }

  // Wi-Fi操作による更新が必要な場合は、ある期間が経過していれば更新する。
  if(update_led_color_flag && (millis() > update_led_tout)){
    update_led_tout = 0;
    update_led_color_flag = false;
    CONF.save();
  }

  // Wifiからの操作
  func_operation_check();
}
