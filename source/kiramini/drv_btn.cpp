
#include "arduino.h"
#include "drv_btn.h"
#include "def.h"

using namespace kira;

// global instance
kira::Button BTN;

#define BTN_PIN       14  // ボタン入力
#define BTN_CHECK_TM  10    // ボタンチェック周期(ms)
#define BTN_LONG_TM   3000  // 長押し時間(ms)

#define STS_IDLE    0
#define WAIT_ON     1
#define WAIT_OFF    2
#define WAIT_OFF2   3
#define WAIT_READ   4

void (Button::*Button::pfnTable[][2])(void) = { 
  {&Button::idle_on,   &Button::nop},
  {&Button::waiton_on, &Button::waiton_off},
  {&Button::waitoff_on,&Button::waitoff_off},
  {&Button::nop,       &Button::waitoff2_off},
  {&Button::nop,       &Button::nop}
};

void Button::idle_on(void)
{
  stime = millis();
  sts = WAIT_ON;
}

void Button::waiton_off(void)
{
  sts = STS_IDLE;
}

void Button::waiton_on(void)
{
  btn = On;
  sts = WAIT_OFF;
}

void Button::waitoff_off(void)
{
  btn = Pushed;
  sts = WAIT_READ;
}

void Button::waitoff_on(void)
{
  if(millis() - stime > BTN_LONG_TM){
    sts = WAIT_OFF2;  
  }
}

void Button::waitoff2_off(void)
{
  USE_SERIAL.printf("long pushed wait\n");

  btn = LongPushed;
  sts = WAIT_READ;    
}

// 初期化処理を行う関数
// コンストラクタは　クラス名::クラス名と同じ名前で構成します
Button::Button()
{
  clear();
}

void Button::setup(void)
{
  USE_SERIAL.printf("btn setup\n");
  // ボタン
  pinMode(BTN_PIN, INPUT);
}

void Button::loop(void)
{
  static unsigned long tm_old = 0;
  unsigned long tm_now = millis();

  // 10ms を超える時間経過毎にチェックする。
  if(tm_now - tm_old > BTN_CHECK_TM){
    int nFac = digitalRead(BTN_PIN);
    
    (this->*pfnTable[sts][nFac])(); 
    
    tm_old = tm_now;
  }

  //USE_SERIAL.printf("btn sts=%d\n", sts);
  //USE_SERIAL.printf(sts);
}

bool Button::is_pushed(void)
{
  return digitalRead(BTN_PIN) == 0;
}

eButtonStatus Button::get(void)
{
  //USE_SERIAL.printf("btn get %d \n", btn);

  return btn;  
}

void Button::clear(void)
{
  USE_SERIAL.printf("btn clear \n");

  btn = Off;
  sts = STS_IDLE;
  stime = 0;
}

