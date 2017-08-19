/*
 * kira_wsremo.ino
 * Wi-Fi(Websocket)でのリモコン処理
 * 
 * WebSocketの送受信処理
 * 
 */
#include <Arduino.h>
#include "def.h"
#include <FS.h>
#include "image.h"
#include "drv_wss.h"
#include "drv_btn.h"
#include "drv_axis3.h"
#include "drv_barled.h"
#include "webserver.h"
#include "config.h"
#include "comlib.h"
#include "webcommon.h"
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson
#include "WebSockets.h"

using namespace kira;

static bool  ws_enabled = false;
static OpMode_t ws_op = OP_NON;

void ws_remo_loop(void)
{
  if(ws_enabled){
    WSS.loop();
  }
}

// リモコン操作可否
void ws_remo_enabled(bool en)
{
  ws_enabled = en;
  
  if(ws_enabled){
    WSS.setup(WSS_OnEvent);  
  }else{
    // WiFi 停止設定
    WebCommon::stopWifi();    
  }
}

// モード変化チェック
void func_operation_check(void)
{
  // 変更されている場合は、残像表示モードへ移行
  if(ws_op != OP_NON){
    kr_img_start();
  }
}

// WebSocetによる操作
OpMode_t kr_remo_get_operation(void)
{
  OpMode_t ret = ws_op;
  ws_op = OP_NON;
  return ret;
}

//// WebSocketによる操作実施
//void func_operation_ws(void)
//{
//  if(ws_op_dir != 0){  
//    // 進む
//    if(ws_op_dir == 1){
//      if(!IMG.load_next()){
//        IMG.load();
//      }
//    // 戻る
//    }else{
//      if(!IMG.load_before()){
//        IMG.load();
//      }
//    }
//    ws_op_dir = 0;
//  }
//
//  if(ws_op_repeat != -1){
//    // 単発
//    if(ws_op_repeat == 0){
//      internal_mode = WAIT_LEFT;
//    // 連続
//    }else{
//      internal_mode = MODE_REPEAT;
//    }
//    ws_op_repeat = -1;
//  }
//}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Websocket系
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// WebSocketの切断イベント処理
void WSS_OnEvent_Disconnected(uint8_t num, uint8_t * payload, size_t length)
{
  USE_SERIAL.printf("[%u] Disconnected!\n", num);
}

// WebSocketの接続イベント処理
void WSS_OnEvent_Connected(uint8_t num, uint8_t * payload, size_t length)
{
  IPAddress ip = WSS.getip(num);
  USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

  // システム設定を接続時に返却する。
  String setting;
  CONF.printTo(setting);
  WSS.send(num, setting);
}

// WebSocketのテキスト受信イベント処理
void WSS_OnEvent_Text(uint8_t num, uint8_t * payload, size_t length)
{
  //USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);

  // JSON用の固定バッファを確保。
  StaticJsonBuffer<200> jsonBuffer;

  // JsonObjectはJSONBufferに格納、パースされた結果を参照保持する。
  // データを参照する場合はこのJsonObjectを見に行く。
  // メモリはスコープから出た際に開放される。
  JsonObject& root = jsonBuffer.parseObject(payload);

  // パースが成功したか確認。できなきゃ終了
  if (root.success()) {
    String tp = root["type"];

    // RGB色指定
    if(tp == "rgb"){
      // データの取り出しは暗黙の型変換
      CONF.rgb[0] = root["color"][0];  // red
      CONF.rgb[1] = root["color"][1];  // green
      CONF.rgb[2] = root["color"][2];  // blue

      update_led_color_flag = true;
      kr_color_start(true);

    // 操作
    }else if(tp == "opr"){
      int dir = root["dir"];
      int repeat = root["repeat"];

      // 進む
      if(dir == 1){
        ws_op = OP_NEXT;
      // 戻る
      }else if(dir == -1){
        ws_op = OP_BEFORE;
      // 単発
      }else if(repeat == 0){
        ws_op = OP_MANUAL;
      // 連続
      }else if(repeat == 1){
        ws_op = OP_REPEAT;
      }

    // ビットマップ指定
    }else if(tp == "bmp"){
      // データの取り出しは暗黙の型変換
      String fname = root["name"];
      long sz = root["size"];

      kr_str_write_start(fname, sz);

    // 文字列バイナリ指定
    }else if(tp == "str"){

      // 表示は消灯する。
      LED.turn_off();

      // jsonファイルは、ファイルとして書き込み保存する。
      File f = SPIFFS.open("/str/str.json", "w");

      // ファイル書き込み
      root.printTo(f);
      f.close();
  
      // debug
      USE_SERIAL.println((char*)payload);
      USE_SERIAL.print("length: ");
      USE_SERIAL.println(length);

       // データの取り出しは暗黙の型変換
      String fname = root["name"];
      long sz = root["size"];

      kr_str_write_start(fname, sz);

    // クリア
    }else if(tp == "clear"){
      // データの取り出しは暗黙の型変換
      String folder = root["folder"];

      func_clear_folder(folder);
      
    }
    
  }else{
    USE_SERIAL.printf("json parse error.\n");
  }
}

// フォルダ内ファイル削除
void func_clear_folder(String folder)
{
  IMG.clear_folder(folder.c_str());
}

void WSS_OnEvent_Unknown(uint8_t num, uint8_t * payload, size_t length)
{
  USE_SERIAL.printf("[%u] Unknown\n", num);
}

// WebSocketコマンド受信イベントハンドラ
void WSS_OnEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
//  USE_SERIAL.printf("event\n", num);

  if(type == WStype_DISCONNECTED){
    WSS_OnEvent_Disconnected(num, payload, length);
  }else if(type == WStype_CONNECTED){
    WSS_OnEvent_Connected(num, payload, length);
  }else if(type == WStype_TEXT){
    WSS_OnEvent_Text(num, payload, length);
  }else if(type == WStype_BIN){
    WSS_OnEvent_Bin(num, payload, length);
  }else {
    WSS_OnEvent_Unknown(num, payload, length);
  }
}
