/*
 * drv_wss.h
 * Webソケット通信クラス
 * 
 * 
 * 
 */
#ifndef websocket_h
#define websocket_h
//#include "arduino.h"
//#include "def.h"
//#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include "WebSockets.h"

namespace kira {

// クラスの定義
// クラス名・コンストラクタ名・関数名や使用する変数名を定義します。
class WebSocket
{
  public:
    WebSocket() ;
    void setup(WebSocketsServer::WebSocketServerEvent pfunc);
    void loop(void);
    bool send(uint8_t num, String & payload);
    IPAddress getip(uint8_t num);

  private:
    void event(uint8_t num, WStype_t type, uint8_t * payload, size_t len);


};

} // namespace kira

extern kira::WebSocket WSS;

#endif  // drv_wss.h
