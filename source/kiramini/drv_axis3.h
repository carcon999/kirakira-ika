/*
 * drv_axis3.h
 * 3軸加速度ドライバ
 * 
 * MMA7660ライブラリを利用して、3軸加速度値をアプリで利用し易い形で
 * 提供するドライバクラス
 * 
 */
 #ifndef drv_axis3_h
#define drv_axis3_h

namespace kira {

#define TILT_LEFT   (0x01)
#define TILT_RIGHT  (0x02)
#define TILT_DOWN   (0x05)
#define TILT_UP     (0x06)

enum eDirection{
  NON = 0,
  LEFT,
  RIGHT,
};

typedef enum eDirection Direction_t;

// クラスの定義
// クラス名・コンストラクタ名・関数名や使用する変数名を定義します。
class Axis3
{
  public:
    Axis3() ;
    void setup(void);
    void get_axis(int8_t* px, int8_t* py, int8_t* pz);
    uint8_t get_tilt(void);
    // 振り方向の取得
    eDirection shake_dir(void);

} ;

} // namespace kira

extern kira::Axis3 AX3;

#endif  // drv_axis3_h
