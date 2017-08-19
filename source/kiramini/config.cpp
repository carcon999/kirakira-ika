#include <arduino.h>
#include <FS.h>          //this needs to be first, or it all crashes and burns...
#include "config.h"

using namespace kira;

// global instance
kira::Config CONF;

// 設定ファイル名
#define JSON_FILE       "/config/config.json"

// constructor
Config::Config()
{
  clear();
}

bool Config::load(void)
{
  bool ret = false;

  //read configuration from FS json
  USE_SERIAL.println("mounting FS...");

  if (SPIFFS.begin()) {
    USE_SERIAL.println("mounted file system");

    // JSONファイル有無判定
    if (SPIFFS.exists(JSON_FILE)) {

      //file exists, reading and loading
      USE_SERIAL.println("reading config file");
      File configFile = SPIFFS.open(JSON_FILE, "r");
      if (configFile) {
        USE_SERIAL.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) 
        {
          USE_SERIAL.println("\nparsed json");
          update = atoi(json["update"]);
          wifi   = atoi(json["wifi"]);
          bright = atoi(json["bright"]);
          length = atoi(json["length"]);
          rgb[0] = atoi(json["rgb"][0]);
          rgb[1] = atoi(json["rgb"][1]);
          rgb[2] = atoi(json["rgb"][2]);
          ret = true;
        } else {
          USE_SERIAL.println("failed to load json config");
        }
      }

    // Jsonファイルなし
    }else{
      USE_SERIAL.println("nothig config file");      
    }
  } else {
    USE_SERIAL.println("failed to mount FS");
  }
  //end read

  return ret;
}

void Config::save(void)
{
  //save the custom parameters to FS
  USE_SERIAL.println("saving config");
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  put_parameter(json);

  File configFile = SPIFFS.open(JSON_FILE, "w");
  if (!configFile) {
    USE_SERIAL.println("failed to open config file for writing");
  }

  json.printTo(configFile);
  configFile.close();
  //end save
}

void Config::put_parameter(JsonObject& json)
{
  json["type"] = "config";
  json["wifi"] = wifi;
  json["update"] = update;
  json["bright"] = bright;
  json["length"] = length;

  JsonArray& data = json.createNestedArray("rgb");
  data.add(rgb[0]);
  data.add(rgb[1]);
  data.add(rgb[2]);
}

void Config::default_save(void)
{
  clear();
  save();
}

void Config::clear(void)
{
  update = 1;       // 更新フラグ
  wifi = 1;         // WiFi利用（WebSocket)
  bright = 32;      // デフォルトの明るさ
  length = 32;      // LEDの長さ

  // 初期色（赤）
  rgb[0] = bright;
  rgb[1] = 0;
  rgb[2] = 0;
}

void Config::printTo(String& str)
{
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  put_parameter(json);
  json.printTo(str);
}

