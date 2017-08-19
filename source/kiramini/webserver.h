/*
 * webserver.h
 * Webサーバクラス
 * 
 * 設定用のWebサーバーの立ち上げと、ブラウザとのやり取りを実施
 * 応答用のHTMLは、index.htmとしてDataフォルダ内に配置している。
 * 
 */
#ifndef webserver_h
#define webserver_h
#include "arduino.h"

namespace kira {

class WebServer
{
  public:
    WebServer();
    void setup(void);
    void loop(void);

  private:
    bool handleFileRead(String path);
    void handleFileNotFound(void);
    void handleFileEdit(void);
    void handleFileUpload(void);
    void handleFileDelete(void);
    void handleFileCreate(void);
    void handleFileList(void);
    void handleSettings(void);
    void handleGetConfig(void);

  private:
    String getContentType(String filename);
    void sendResult(int code, String msg);
    void resultSuccess(void);
    void imageUpdated(void);
} ;

} // namespace kira

extern kira::WebServer WEB;

#endif  // webserver_h
