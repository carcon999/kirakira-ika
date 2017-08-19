
#include "arduino.h"
#include "MMA7660.h"
#include "drv_axis3.h"

using namespace kira;

// global instance
kira::Axis3 AX3;

MMA7660 accelemeter;

#define DEF_BUFF_SHAKE_SIZE  4    // 2 / 4 / 8 / 16
#define DEF_SHAKE_MASK       (DEF_BUFF_SHAKE_SIZE - 1)

#define USE_SERIAL Serial

const int8_t t2comp[64] = {
  0,1,2,3,4,5,6,7,8,9,10,11,12,13,
  14,15,16,17,18,19,20,21,22,23,24,25,26,27,
  28,29,30,31,-32,-31,-30,-29,-28,-27,-26,-25,-24,-23,
  -22,-21,-20,-19,-18,-17,-16,-15,-14,-13,-12,-11,-10,
  -9,-8,-7,-6,-5,-4,-3,-2,-1
};

// I2C割り込み
static void i2c_interrupt(void)
{
}

// 初期化処理を行う関数
// コンストラクタは　クラス名::クラス名と同じ名前で構成します
Axis3::Axis3()
{
}

void Axis3::setup(void)
{
  accelemeter.init();
  accelemeter.setSampleRate(AUTO_SLEEP_120);

  delay(10);
  // dammy read
  get_tilt();
  delay(10);
  get_tilt();
  delay(10);

  // I2C    SDA CLK
//  Wire.begin(SDA_PIN, CLK_PIN);
//  attachInterrupt(INT_PIN, i2c_interrupt, FALLING); //割り込みを設定する
}

void Axis3::get_axis(int8_t* px, int8_t* py, int8_t* pz)
{
  int8_t x,y,z;
  int val = t2comp[y];
  
  accelemeter.getXYZ(&x, &y, &z);

  *px = t2comp[x];
  *py = t2comp[y];
  *pz = t2comp[z];
}

// 
uint8_t Axis3::get_tilt(void)
{
  uint8_t ret = 0;
  MMA7660_DATA dt;
  
  for(int i = 0 ; i < 3 ; i++){
    if(accelemeter.getAllData(&dt))
    {
      ret = (dt.TILT >> 2) & 0x07;
      break;
    }
    delay(10);
  }

  return ret;
}

eDirection Axis3::shake_dir(void)
{
  static int shake_log[DEF_BUFF_SHAKE_SIZE];    // 取り込み値保持バッファ
  static int shake_sub[DEF_BUFF_SHAKE_SIZE];    // 取り込み差分バッファ
  static int shake_sum = 0;                     // 差分加算合計値
  static uint8_t shake_index_old = 0; 
  static uint8_t shake_index = 1;

  Direction_t ret = NON;  
  int8_t x,y,z;

//  USE_SERIAL.print("getXYZ start ");
  accelemeter.getXYZ(&x,&y,&z);
//  USE_SERIAL.println(y);
//  USE_SERIAL.println("end");

  int val = t2comp[y];

//  USE_SERIAL.println(val);

  shake_log[shake_index] = val;
  shake_index++;
  shake_index &= DEF_SHAKE_MASK;
  
  shake_sum -= shake_sub[shake_index_old];
  shake_sub[shake_index_old] = shake_log[shake_index_old] - val;
  shake_sum += shake_sub[shake_index_old];

  shake_index_old++;
  shake_index_old &= DEF_SHAKE_MASK;
  
  // 右方向への操作待ち
  if(shake_sum > 8 ){
    ret = LEFT;
    //USE_SERIAL.println("LEFT");
  }
  // 左方向への操作待ち
  else if(shake_sum < - 5 ){
    ret = RIGHT;
    //USE_SERIAL.println("RIGHT");
  }

  return ret;
}



