var str_color_index = 0;
var mycanvas;

function msg_get_canvas(){
  if(mycanvas == null){
    mycanvas = document.createElement('canvas');
    mycanvas.id = "mycanvas";
    mycanvas.height = 32;
    mycanvas.width = 1024;
  }

  return mycanvas;
}

// 色指定
function msg_selected_color(color){
  str_color_index = color;

  // 画面のテキスト色を変更
  document.getElementById('strint_txt').style.color = msg_get_index_color(color);  
}

// 色の取得処理
function msg_get_index_color(index){
  if(index == 0)      return 'rgb(192, 192, 192)';  // 白
  else if(index == 1) return 'rgb(51,  122, 183)';  // 青
  else if(index == 2) return 'rgb(92,  184, 92)';   // 緑
  else if(index == 3) return 'rgb(91,  192, 222)';  // 水
  else if(index == 4) return 'rgb(240, 173, 78)';   // 橙
  else if(index == 5) return 'rgb(217, 83, 79)';    // 赤
  else                return 'rgb(192, 192, 192)';  // 白
}

// キャンバスに文字列を描画する。
function msg_draw_canvas(txt) {
  var canvas = msg_get_canvas();
  var context = canvas.getContext('2d');


  if ( sys_led_length == 22) {
    context.font = "21px Ludica Sans Unicode";
  } else {
    context.font = "31px Ludica Sans Unicode";
  }
  context.fillStyle = "white"; //塗り潰し色を白に
  context.clearRect(0, 0, canvas.width, canvas.height);   // キャンバスのクリア

  // １文字単位で描画する。
  for (var i = 0; i < txt.length; i++) {
    if ( sys_led_length == 22) {
      context.fillText(txt[i],i*22, 29, 21);      //テキストを塗り潰しで描画
    }else{
      context.fillText(txt[i],i*32, 27, 31);      //テキストを塗り潰しで描画
    }
  }

  return txt.length;
}

// 文字の表示バイナリデータを生成する。
function msg_create_string_bin(str_length){

  // 全体のデータサイズ
  var buffer = new ArrayBuffer(str_length * sys_led_length * 4);

  // 画像を作る処理
  var img = new Uint32Array(buffer);
  {
    var canvas = msg_get_canvas();
    var context = canvas.getContext('2d');
    var imageData = context.getImageData(0, 0, canvas.width, canvas.height);
    var pixels = imageData.data;  // ピクセル配列：4要素で1ピクセル
    var width = imageData.width, height = imageData.height;
    var index = 0;

    // 縦にスキャンする
    for (var x = 0; x < str_length * sys_led_length; x++) {
      for (var y = 0; y < height; y++) {
        var base = (y * width + x) * 4;
        var sum = pixels[base + 0] + pixels[base + 1] + pixels[base + 2];

        img[index] <<= 1;
        // データがある場合
        if(sum > 0){
          img[index] |= 0x01;
        }
      }
      index++;
    }
  }

  return( new Uint32Array(buffer));

}

// ビットマップデータを生成する。
// 24bitフルカラーデータで作る
function msg_create_bitmap(str_length){
  var header = new Uint8Array([0x42, 0x4D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 
                               0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00, 
                               0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00]);

  // 全体のデータサイズ
  var w = 32;
  var h = str_length * sys_led_length;
  var sz = header.length + (w * h * 3);
  var buffer = new ArrayBuffer(sz);
  
  var view = new DataView(buffer);

  // ヘッダーの基本データをコピーする
  var ary_u8 = new Uint8Array(buffer, 0, header.length);
  ary_u8.set(header);

  // ファイルサイズ
  view.setUint32(2, sz, true);

  // 画像の幅
  view.setUint32(18, w, true);

  // 画像の高さ
  view.setUint32(22, h, true);

  // 画像を作る処理
  var img = new Uint8Array(buffer, header.length, h * w * 3);
  {
    var canvas = msg_get_canvas();
    var context = canvas.getContext('2d');
    var imageData = context.getImageData(0, 0, canvas.width, canvas.height);
    var pixels = imageData.data;  // ピクセル配列：4要素で1ピクセル
    var width = imageData.width, height = imageData.height;

    var grb = msg_create_color_grb();

    // 縦にスキャンする
    for (var x = 0; x < width; x++) {
      for (var y = 0; y < height; y++) {
        var base = (y * width + x) * 4;
        var sum = pixels[base + 0] + pixels[base + 1] + pixels[base + 2];
        // データがある場合
        if(sum > 0){
          var pos = (x * w + (w - 1 - y) )* 3;
          img[pos + 0] = grb[0];  // GREEN
          img[pos + 1] = grb[1];  // RED
          img[pos + 2] = grb[2];  // BLUE
        }
      }
    }
  }

  return( new Uint8Array(buffer));
}

// ビットマップ用の色を生成
function msg_create_color_grb(){
  if(str_color_index == 0)      return [sys_bright, sys_bright, sys_bright];  // 白
  else if(str_color_index == 1) return [0, 0, sys_bright];                    // 青
  else if(str_color_index == 2) return [sys_bright, 0, 0];                    // 緑
  else if(str_color_index == 3) return [sys_bright, 0, sys_bright];           // 水
  else if(str_color_index == 4) return [sys_bright/2, sys_bright, 0];         // 橙
  else if(str_color_index == 5) return [0, sys_bright, 0];                    // 赤
  else                          return [sys_bright, sys_bright, sys_bright];  // 白
}
