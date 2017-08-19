/*
 * webcommon.h
 * 無線共通クラス
 * 
 * Wi-Fiの汎用処理
 * 
 */
#ifndef webcommon_h
#define webcommon_h

namespace kira {

// クラスの定義
// クラス名・コンストラクタ名・関数名や使用する変数名を定義します。
class WebCommon
{
  public:
    static void startSoftAp(void);
    static void stopWifi(void);
    static char* apName(void);

  private:
    WebCommon();
};

} // namespace kira

#endif  // webcommon_h
