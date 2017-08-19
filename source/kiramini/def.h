/*
 * def.h
 * マクロ定義
 * 
 * システムの定数定義する。
 */

#ifndef DEF_H
#define DEF_H

#define null         NULL
#define LED_MAX_LENGTH    (32)
#define LED_RGB_SIZE      (3)

// UARTデバッグ出力をする場合は、trueにする。
#define UART_DEBUG false

#define USE_SERIAL Serial

#define LED_PIN       12  // チップLED出力

#define ESP_PIN       15  // ESP_PIN固有
#define SDA_PIN        4  // I2CのSDA
#define CLK_PIN        5  // I2CのCLK
#define INT_PIN       13  // I2Cの割り込み

// LEDの送信時間＋RESETを含めた時間
// 1.2us * 24bti * 32LED = 921.6us + 80us = 1001.6us　以上が必要
// 内部の処理時間やDMAのオーバヘッドを加味すると倍は必要
#define LED_DELAY     2000    // LEDの送信周期(us)

#define BRIGHT_MAX    (192)    // 最大明るさ

enum eWaitMode{
  RIGHT_WAIT,
  LEFT_WAIT
};

// 操作定義
enum eOperation{
  OP_NON,     // 操作なし
  OP_NEXT,    // 次
  OP_BEFORE,  // 戻る
  OP_MANUAL,  // 手動
  OP_REPEAT   // 繰り返し
};

enum eAppMode{
  APPMODE_SET,
  APPMODE_COLOR,
  APPMODE_STR,
  APPMODE_IMG,

  APPMODE_NUM
};

typedef enum eWaitMode WaitMode_t;
typedef enum eAppMode AppMode_t;
typedef enum eOperation OpMode_t;

typedef void (*FUNCPTR)(void);

#endif // #ifndef DEF_H
