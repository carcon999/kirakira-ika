/*
 * kira_image.ino
 * 画像表示処理
 * 
 * 残像イメージ表示処理
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

using namespace kira;

enum eModeImage{
  IMG_WAIT_LEFT,
  IMG_PROC_LEFT,
  IMG_WAIT_RIGHT,
  IMG_PROC_RIGHT,
  IMG_REPEAT,        // 繰り返しモード  
  IMG_MODE_NUM
};
typedef enum eModeImage ModeImg_t;

// 動作モード
static ModeImg_t mode_img= IMG_WAIT_LEFT;

typedef void (*FUNCPTR)(void);


const FUNCPTR tImgFunc[IMG_MODE_NUM][4] = {
  {func_check_key,  wait_left,    kr_img_nop,        kr_img_nop},
  {kr_img_nop,        kr_img_nop,     kr_img_nop,        proc_left },
  {func_check_key,  kr_img_nop,     wait_right,      kr_img_nop},
  {kr_img_nop,        kr_img_nop,     kr_img_nop,        proc_right},
  // 残像表示処理（繰り返し）
  {kr_img_nop,        kr_img_nop,     kr_img_nop,        func_repeat },  
};

void loop_image(void)
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
  (*tImgFunc[mode_img][fac])();

  BTN.loop();
}

void kr_img_start(void)
{
  LED.turn_off();
  // 画像ファイルアクセス初期化
  if(IMG.setup(IMAGE)){
    mode_img = IMG_WAIT_LEFT;  
    // 画像モードへ移行
    gAppMode = APPMODE_IMG;
  }else{
    kr_color_start(false);
  }

  gTimeout = micros() + LED_DELAY;
}

static bool kr_img_drawline(int dir)
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

void wait_left(void)
{
  mode_img = IMG_PROC_LEFT;
}
void proc_left(void)
{
  if(!kr_img_drawline(1)){
    mode_img = IMG_WAIT_RIGHT;
  }
}
void end_left(void)
{
  mode_img = IMG_WAIT_RIGHT;
}
void wait_right(void)
{
  mode_img = IMG_PROC_RIGHT;
}
void proc_right(void)
{
  if(!kr_img_drawline(-1)){
    mode_img = IMG_WAIT_LEFT;
  }  
}
void end_right(void)
{
  mode_img = IMG_WAIT_LEFT;
}

// キー入力チェック
void func_check_key(void)
{
  eButtonStatus sts = BTN.get();

  if(sts == Pushed){
    BTN.clear();

    // 全画像を表示したら、単色表示へ戻す。
    if(!IMG.load_next()){
      // 単色表示モード
      kr_color_start(false);
    }
  }else if(sts == LongPushed){
    
    BTN.clear();
    mode_img = IMG_REPEAT;
  }

  // Wi-Fiリモコンの操作変更
  kr_img_operation();
}

void func_repeat(void)
{
  uint8_t buff[LED_MAX_LENGTH][LED_RGB_SIZE];

  int ret = IMG.get_repeat_line(buff);  

  LED.set_color(buff);

  if(!ret){
    eButtonStatus sts = BTN.get();

    if(sts == Pushed){
      BTN.clear();
      if(!IMG.load_next()){
        // 単色表示モード
        kr_color_start(false);
      }
    }else if(sts == LongPushed){
      BTN.clear();
      mode_img = IMG_WAIT_LEFT;  
    }

    // Wi-Fiリモコンの操作変更
    kr_img_operation();
  }
}

static void kr_img_nop(void)
{
}

static void kr_img_operation(void)
{
  // Wi-Fiリモコンの操作変更
  OpMode_t op = kr_remo_get_operation();

  if(op != OP_NON)
  {
    if(op == OP_NEXT){
      if(!IMG.load_next()){
        IMG.load();
      }
    }else if(op == OP_BEFORE){
      if(!IMG.load_before()){
        IMG.load();
      }
    }else if(op == OP_MANUAL){
      mode_img = IMG_WAIT_LEFT;
      
    }else if(op == OP_REPEAT){
      mode_img = IMG_REPEAT;
    }
  }
}

