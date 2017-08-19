/*
 * comlib.h
 * プロジェクト内汎用処理
 * 
 * 汎用的に各クラスから利用できそうな処理を定義 
 * 
 */
#ifndef comlib_h
#define comlib_h

#include "def.h"

#define CSV_MAX     (10)

namespace kira {

class ComLib
{
  public:
    static void delay_and_led(int ms, int blink);
    static void reset(void);
    static int splitCsv(String data, String (&dst)[CSV_MAX]);
    static String formatBytes(size_t bytes);
    static void get_index_to_grb(uint8_t br, uint8_t index, uint8_t grb[LED_RGB_SIZE]);
    static const char* get_master_img_folder(void);
  private:
    ComLib(){};
};

} // namespace kira

#endif  // comlib_h
