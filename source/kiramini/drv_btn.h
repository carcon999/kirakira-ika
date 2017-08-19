/*
 * drv_btn.h
 * ボタンドライバ
 * 
 * タクトスイッチのボタン検出ドライバクラス
 * 短押し、長押しを検出する。
 * 
 */
#ifndef drv_btn_h
#define drv_btn_h

namespace kira {

// Buttonの状態
enum eButtonStatus{
  Off = 0,    // なし
  On,         // 押下中
  Pushed,     // 押下
  LongPushed, // 長押し（３秒以上）
};

// クラスの定義
// クラス名・コンストラクタ名・関数名や使用する変数名を定義します。
class Button
{
  public:
    Button() ;
    void setup(void);
    void loop(void);
    bool is_pushed(void);
    eButtonStatus get(void);
    void clear(void);
    
  private:
    static void (Button::*pfnTable[][2])(void);
    void nop(void){};
    void idle_on(void);
    void waiton_off(void);
    void waiton_on(void);
    void waitoff_off(void);
    void waitoff_on(void);
    void waitoff2_off(void);
    
  private:
    eButtonStatus btn;
    uint8_t sts;
    unsigned long stime; // 押下開始時刻
} ;

}  // namespace kira

extern kira::Button BTN;

#endif  // drv_btn_h
