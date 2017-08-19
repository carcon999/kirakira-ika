#include "arduino.h"
#include <ESP8266WiFi.h>
#include "comlib.h"
#include "webcommon.h"

using namespace kira;

static const char *ssid_prefix = "KIRA-";
static const char *pass = "kirakira123";

static IPAddress my_ip( 192, 168, 10, 1 );
static IPAddress subnet( 255, 255, 255, 0 );

#define DBG_OUTPUT_PORT Serial

// global instance
//kira::WebCommon WC;

WebCommon::WebCommon()
{
}

void WebCommon::stopWifi(void)
{
  WiFi.softAPdisconnect(true);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);  

  // WiFi設定待ち
  ComLib::delay_and_led(3000, 250);
}

void WebCommon::startSoftAp(void)
{
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(my_ip, my_ip, subnet);

  // WiFi設定待ち
  ComLib::delay_and_led(3000, 250);

  DBG_OUTPUT_PORT.println(apName());
  WiFi.softAP(apName(), pass);
  IPAddress myIP = WiFi.softAPIP();
  DBG_OUTPUT_PORT.print("AP IP address: ");
  DBG_OUTPUT_PORT.println(myIP);
}

// アクセスポイント名
char* WebCommon::apName(void)
{
  static char buff[20];
  byte mac_addr[6];
  WiFi.macAddress(mac_addr);
  sprintf(buff, "%s%02x%02x%02x", ssid_prefix, mac_addr[3], mac_addr[4], mac_addr[5]);  
  return buff;
}


