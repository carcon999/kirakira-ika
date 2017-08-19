/*
 * drv_barled.h
 * LEDバードライバ
 * 
 * I2Sライブラリを利用して、LEDバーの色制御を
 * 提供するドライバクラス
 * 
 */
#ifndef drv_barled_h
#define drv_barled_h
#include "def.h"

namespace kira {

// クラスの定義
// クラス名・コンストラクタ名・関数名や使用する変数名を定義します。
class Barled
{
  public:
    Barled() ;
    // 初期化
    void begin(uint8_t len);
    // 停止
    void end(void);
    // 表示更新データ転送（DMA)
    void update(void);
    // 全データ配列指定
    void set_color(uint8_t grb[LED_MAX_LENGTH][LED_RGB_SIZE]);
    // 個別データ指定
    void set_color(int pos, uint8_t grb[LED_RGB_SIZE]);
    // 一括同色指定（BRG)
    void set_color(uint8_t grb[LED_RGB_SIZE]);
    // 一括同色指定（HSV)
    void set_color(int hue, uint8_t sat, uint8_t val);
    // クリア指定
    void set_color_clear(void);
    // 点灯（段階）
    void turn_on(uint8_t grb[LED_RGB_SIZE]);
    // 消灯
    void turn_off(void);
    // テスト
    void test(void);
    // 進捗表示
    void set_progress(uint8_t br, uint8_t n);

  private:

    static void hsv_to_rgb(int h, int s, int v, uint8_t grb[LED_RGB_SIZE]);
    uint32_t led_count;
    uint32_t buff[LED_MAX_LENGTH][LED_RGB_SIZE];
} ;

} // namespace kira

extern kira::Barled LED;

#endif  // drv_barled_h
