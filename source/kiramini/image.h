/*
 * image.h
 * 画像管理クラス
 * 
 * 画像ファイルのロードや変換処理を実施する。
 * 
 */
#ifndef image_h
#define image_h
#include "arduino.h" 
#include "def.h"
#include <FS.h>

namespace kira {

enum eWorkFolder{
  IMAGE,      // 画像フォルダ
  STRING,     // 文字フォルダ
  
  WORK_FSIZE  // 合計サイズ
};

typedef enum eWorkFolder eFolder_t;

typedef struct bmp_inf{
  char filename[16];// ファイル名
  uint32_t offset;  // オフセット
  uint16_t padding; // パディング量
  uint16_t bitcount;// ビットカウント
  uint16_t width;   // 画像の幅
  uint16_t height;  // 画像の高さ
  uint16_t linesize;// １行サイズ
}BMP_INF;

typedef struct load_inf{
  int index;
  int file_count;   // ファイル数
}FILE_INF;

typedef void (*CALLBACK)(void);

// クラスの定義
// クラス名・コンストラクタ名・関数名や使用する変数名を定義します。
class Image
{
  public:
    Image() ;
    bool setup(eFolder_t f);
    bool load(void);
    bool load_top(void);
    bool load_next(void);
    bool load_before(void);
    bool get_next_line(int dir, uint8_t buff[LED_MAX_LENGTH][LED_RGB_SIZE]);
    bool get_repeat_line(uint8_t buff[LED_MAX_LENGTH][LED_RGB_SIZE]);
    bool create_rotation_file(uint8_t bright, int index);
    void export_use_file(uint8_t bright, bool bForce, CALLBACK pFunc);
    void close(void);
    const char* get_work_folder_name(eFolder_t f);
    void clear_folder(const char* name);
    void create_string_file(uint8_t bright);
    
  private:
    void set_range(int len);
    bool open_bitmap_file(char* name);
    void convert_brightness(uint8_t rgb[LED_RGB_SIZE], float offset);
    void rgb_to_hsv(uint8_t rgb[LED_RGB_SIZE], float* H, float* S, float* V);
    void hsv_to_rgb(float H, float S, float V, uint8_t rgb[LED_RGB_SIZE]);
    void clear(void);
    void create_string_bitmap(File f, uint8_t br, uint8_t led_index, int fno, int sz, int led_sz);
 
  private:
    BMP_INF inf;    // ビットマップデータ情報
    FILE_INF finf;  // ファイル名情報
    File file;      // ビットマップファイル
    int len;        // LED長さ
    eFolder_t ft;   // フォルダタイプ
} ;

} // namespace kira

extern kira::Image IMG;

#endif  // Image_h
