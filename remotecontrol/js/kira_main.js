const STRING_SIZE = 8;// 文字列サイズ

// 起動ロード処理
function main_onload(){
  ws_Connect();
}

// メッセージデータの転送処理
function main_OnClick_transfer_button() {

  // 半角英数→全角変換
  var txt = document.getElementById('strint_txt').value;

  if(txt.length != 0){

    txt = txt.replace(/[A-Za-z0-9]/g, function(s) {
      return String.fromCharCode(s.charCodeAt(0) + 65248).trim();
    });

    // 区切りを明確にするためにスペースを最後に必ず入れる。
    txt += "　";

    // 描画＋送信処理
    main_request_string_bitmap(txt);
  }
}

function main_request_string_bitmap(txt){

  // 非表示キャンバスに文字列を描画する。
  var str_len = msg_draw_canvas(txt);

  // 描画文字数０の場合は処理しない
  if(str_len != 0)
  {
    // ビットマップを生成する。
    var bin = msg_create_string_bin(str_len);
    var len = bin.byteLength;

    // これから送信するビットマップデータの情報を送信
    ws_request_string("str.bin", len, str_color_index);

// デバッグ用ファイルのダウンロード
//{
//  var blob = new Blob([bin], {type:"application/octet-binary"});
//  var a = document.createElement("a");
//  a.href = URL.createObjectURL(blob);
//  a.target = ('_blank');
//  a.download = "aaa.bin";
//  a.click();
//}
    // ビットマップデータをバイナリ転送
    ws_request_bitmap_bin(bin);
  }
}

// 色ボタンの選択押下
function main_OnClick_color_button(color) {
  msg_selected_color(color);
  main_OnClick_transfer_button();
}

// 音声認識文字の設定と転送
function main_voice_string(txt){
  document.getElementById('strint_txt').value = txt;
  main_OnClick_transfer_button();
}

// 送りボタン押下
function main_OnClick_dir(dir) {
  ws_request_operation(dir, -1);
}

// 連続ボタン押下
function main_OnClick_repeat(repeat) {
  ws_request_operation(0, repeat);
}
