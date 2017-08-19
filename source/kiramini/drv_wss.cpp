
#include "arduino.h"
#include "drv_wss.h"
#include "webcommon.h"
#include "def.h"

#define SOCKET_PORT   9091

using namespace kira;

// global instance
kira::WebSocket WSS;

WebSocketsServer ws = WebSocketsServer(SOCKET_PORT);

// 初期化処理を行う関数
// コンストラクタは　クラス名::クラス名と同じ名前で構成します
WebSocket::WebSocket()
{
}

void WebSocket::setup(WebSocketsServer::WebSocketServerEvent pfunc)
{

  WebCommon::startSoftAp();

  // WebSocket初期設定
  ws.begin();
  ws.onEvent(pfunc);

  USE_SERIAL.println("web socket start!");
}

IPAddress WebSocket::getip(uint8_t num)
{
  return ws.remoteIP(num);
}

bool WebSocket::send(uint8_t num, String & payload)
{
  return ws.sendTXT(num, payload);  
}

void WebSocket::loop(void)
{
  ws.loop();
}

