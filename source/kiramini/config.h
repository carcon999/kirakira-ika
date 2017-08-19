/*
 * config.h
 * 設定クラス
 * 
 * 動作設定をファイルとして記録・管理する。
 * ファイルは、json形式で保存する。
 */

#ifndef CONFIG_H
#define CONFIG_H
#include "def.h"
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson

namespace kira {

class Config
{
public:
  Config();

  // functions
  bool load(void);
  void save(void);
  void default_save(void);
  void printTo(String& str);

  // parameter
  uint8_t update;             // 画像更新フラグ
  uint8_t wifi;               // WiFi利用
  uint8_t bright;             // 明るさ
  uint8_t length;             // LED長さ
  uint8_t rgb[LED_RGB_SIZE];  // 初期色

private:
  void clear(void);
  void put_parameter(JsonObject& json);
  
}; // Config

} // namespace kira

extern kira::Config CONF;

#endif // #ifndef CONFIG_H
