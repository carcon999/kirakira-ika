
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include "webserver.h"
#include "comlib.h"
#include "webcommon.h"
#include "config.h"
#include "def.h"

using namespace kira;

#define USE_SERIAL Serial

// global instance
kira::WebServer WEB;

ESP8266WebServer server(80);
//holds the current upload
File fsUploadFile;

#define DEF_API_LIST          "/list"
#define DEF_API_DEL           "/del"
#define DEF_API_UPLOAD        "/upload"
#define DEF_API_SET           "/set"

// 初期化処理を行う関数
// コンストラクタは　クラス名::クラス名と同じ名前で構成します
WebServer::WebServer()
{
}

// セットアップ
void WebServer::setup(void)
{
  USE_SERIAL.begin(115200);
  USE_SERIAL.print("\n");
  USE_SERIAL.setDebugOutput(true);
  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {    
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      USE_SERIAL.printf("FS File: %s, size: %s\n", fileName.c_str(), ComLib::formatBytes(fileSize).c_str());
    }
    USE_SERIAL.printf("\n");
  }

  WebCommon::startSoftAp();

  //SERVER INIT
  //list directory
  server.on(DEF_API_LIST, HTTP_GET, std::bind(&WebServer::handleFileList, this));

  //load editor
  server.on("/edit", HTTP_GET, std::bind(&WebServer::handleFileEdit, this));

  //create file
  server.on("/edit", HTTP_PUT, std::bind(&WebServer::handleFileCreate, this));

  //delete file
  server.on(DEF_API_DEL, HTTP_GET, std::bind(&WebServer::handleFileDelete, this));

  //led setting
  server.on(DEF_API_SET, HTTP_GET, std::bind(&WebServer::handleSettings, this));

  //upload file
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  server.on(DEF_API_UPLOAD, HTTP_POST, std::bind(&WebServer::resultSuccess, this), std::bind(&WebServer::handleFileUpload, this));

  //file not found
  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound(std::bind(&WebServer::handleFileNotFound, this));

  server.begin();
  USE_SERIAL.println("HTTP server started");

  // config file load.
  CONF.load();
}

void WebServer::imageUpdated(void)
{
  if(!CONF.update){;
    CONF.update = 1;
    CONF.save();
  }
}

void WebServer::loop(void)
{
  server.handleClient();
}

bool WebServer::handleFileRead(String path){
  USE_SERIAL.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void WebServer::handleFileEdit(void){
  if(!handleFileRead("/edit.htm")){
    server.send(404, "text/plain", "FileNotFound");
  }
}

void WebServer::handleFileNotFound(void){
  if(!handleFileRead(server.uri())){
    server.send(404, "text/plain", "FileNotFound");
  }
}

// アップロード
void WebServer::handleFileUpload(void){

  if(server.uri() != DEF_API_UPLOAD) return;

  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = ComLib::get_master_img_folder();
    filename += upload.filename;

    // ファイルが存在したら削除する。
    if(SPIFFS.exists(filename)){
      SPIFFS.remove(filename);
    }

    USE_SERIAL.print("handleFileUpload Name: "); USE_SERIAL.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    //USE_SERIAL.print("handleFileUpload Data: "); USE_SERIAL.println(upload.currentSize);
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile)
      fsUploadFile.close();

    USE_SERIAL.print("handleFileUpload Size: "); USE_SERIAL.println(upload.totalSize);

    // 画像更新フラグON
    imageUpdated();
  }
}

void WebServer::resultSuccess(void){
  sendResult(200, "success");
}

// 結果のJSON返却
void WebServer::sendResult(int code, String msg){
  String res = "[{\"result\":\"" + msg + "\"}]";
  return server.send(code, "text/json", res);
}

// 画像ファイルの削除
// 例
// http://esp8266fs.local/del?name=map.bmp
void WebServer::handleFileDelete(void){
  if(server.uri() != DEF_API_DEL) return;

  if(server.args() == 0){
    sendResult(500, "bad args");
    return;
  }
  String fnames = server.arg(0);
  USE_SERIAL.println("handleFileDelete: " + fnames);

  String dst[CSV_MAX] = {"\0"};
  int sz = ComLib::splitCsv(fnames, dst);

  String path;
  for(int i = 0 ; i < sz ; i++){
    path = ComLib::get_master_img_folder() + dst[i];
    USE_SERIAL.println("handleFileDelete: " + path);
    if(!SPIFFS.exists(path)){
      String msg = dst[i] + " is not found.";
      sendResult(404, msg);
      return;
    }
    SPIFFS.remove(path);
    // 画像更新フラグON
    imageUpdated();
  }

  sendResult(200, "success");
}

// LED設定の実施
// 例
// http://esp8266fs.local/set?prm=32,64,1
void WebServer::handleSettings(void){
  if(server.uri() != DEF_API_SET) return;

  // 引数なしは、パラメータ取得
  if(server.args() == 0){
    USE_SERIAL.println("handleGetConfig");
    handleGetConfig();
    return;
  }

  String params = server.arg(0);
  USE_SERIAL.println("handleSettings: " + params);

  String dst[CSV_MAX] = {"\0"};
  int sz = ComLib::splitCsv(params, dst);

  if(sz != 3){
    sendResult(404, "parameter error.");
    return;
  }

  // LED数のチェック（第一パラメータ）
  int len = atoi(dst[0].c_str());

  USE_SERIAL.print("LED len: ");
  USE_SERIAL.println(len);

  if(len > 0 && len <= LED_MAX_LENGTH){
    CONF.length = len;
  }

  // 明るさのチェック（第二パラメータ）
  int br = atoi(dst[1].c_str());

  USE_SERIAL.print("LED bright: ");
  USE_SERIAL.println(br);

  if(br > 0 && br <= BRIGHT_MAX){
    CONF.bright = br;
  }

  // Wifi利用のチェック（第三パラメータ）
  int wifi = atoi(dst[2].c_str());

  USE_SERIAL.print("Wi-Fi: ");
  USE_SERIAL.println(wifi);

  if(wifi >= 0 && wifi <= 1){
    CONF.wifi = wifi;
  }
  

  CONF.update = 1;
  CONF.save();

  sendResult(200, "success");
}


// 生成
void WebServer::handleFileCreate(void){
  if(server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  USE_SERIAL.println("handleFileCreate: " + path);
  if(path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if(SPIFFS.exists(path))
    return server.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if(file)
    file.close();
  else
    return server.send(500, "text/plain", "CREATE FAILED");
  server.send(200, "text/plain", "");
  path = String();
}

// リスト
// 例
// http://esp8266fs.local/list
void WebServer::handleFileList(void) {
  if(server.uri() != DEF_API_LIST) return;

  String folder = ComLib::get_master_img_folder();
  
  Dir dir = SPIFFS.openDir(folder);

  int offset = String(folder).length();
  String output = "[";
  while(dir.next()){
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"name\":\"";
    output += String(entry.name()).substring(offset);
    output += "\",\"size\":";
    output += String(dir.fileSize());
    output += "}";
    entry.close();
  }
  
  output += "]";
  server.send(200, "text/json", output);
}

// 設定取得
// 例
// http://esp8266fs.local/set
void WebServer::handleGetConfig(void) {
  String json = "";
  CONF.printTo(json);
  server.send(200, "text/json", json);
}


//getContentType
String WebServer::getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}


