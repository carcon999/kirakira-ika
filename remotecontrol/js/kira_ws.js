var websocket = null;              // Webソケット
var sys_led_length = 22;           // LED個数
var sys_bright = 32;               // LED明るさ
const BIN_BLOCK_SIZE = 1024;       // バイナリ送信ブロックサイズ

// WebSocket 接続処理
function ws_Connect() {
  var uri = "ws://" + document.getElementById("ipadder").value;
  if (websocket == null) {
    // 情報表示
    ws_start();
  
    websocket = new WebSocket(uri);
    
    websocket.onopen    = ws_onOpen;
    websocket.onmessage = ws_onMessage;
    websocket.onclose   = ws_onClose;
    websocket.onerror   = ws_onError;
  }
}

function ws_Disconnect() {
  if (websocket != null) {
    websocket.close();
    websocket = null;
  }
}

// Websocket 接続完了イベント
function ws_start() {
  // 情報をOPEN済みに変更する。
  var inf = document.getElementById('information');
  inf.className = "alert alert-info";
  inf.innerHTML = "<strong>接続処理中</strong> しばらくお待ちください。";
}

// Websocket 接続完了イベント
function ws_onOpen(event) {
  // 情報をOPEN済みに変更する。
  var inf = document.getElementById('information');
  inf.className = "alert alert-success";
  inf.innerHTML = "<strong>接続中</strong> 通信できます。";
}

// Websocket メッセージ受信イベント
function ws_onMessage(event) {
  if (event && event.data) {
    var con = JSON.parse(event.data);

    // システム設定を受信した場合
    if(con.type == "config"){
      sys_led_length = con.length;
      sys_bright = con.bright;
    }
  }
}


// Websocket 切断イベント
function ws_onClose(event) {
  if (websocket != null){
    websocket = null;
  }

  // 情報をCLOSE済みに変更する。
  // ただし、エラー表示中のCloseは表示しない。
  var inf = document.getElementById('information');
  if(inf.className != "alert alert-danger"){
    inf.className = "alert alert-warning";
    inf.innerHTML = "<strong>未接続</strong> 再接続する場合は、右上から選択してください。";
  }
}

// Websocket エラー発生イベント
function ws_onError(event) {
  // 情報をエラーに変更する。
  var inf = document.getElementById('information');
  inf.className = "alert alert-danger";
  inf.innerHTML = "<strong>エラー</strong> 通信エラーが発生しました。接続を確認してください。接続の設定は右上にあります。";
}

// RGB色変更要求
function ws_request_color(rgb) {
  if(websocket != null)
  {
    websocket.send('{"type":"rgb","color":[' + rgb[0] + "," + rgb[1] + "," + rgb[2] + ']}');
  }
}

// RGB色変更要求
function ws_request_operation(dir, repeat) {
  if(websocket != null)
  {
    websocket.send('{"type":"opr","dir":' + dir + ',"repeat":' + repeat  + '}');
  }
}


// フォルダクリア送信要求
function ws_request_clear_folder(folder) {
  if(websocket != null)
  {
    websocket.send('{"type":"clear","folder":"' + folder + '"}');
  }
}
// 文字バイナリ送信要求
function ws_request_string(fname, len, index, unit) {
  if(websocket != null)
  {
    var s = '{"type":"str","pallet":' + index + ',"name":"' + fname + '","size":' + len +',"leds":' + sys_led_length + '}';
    websocket.send(s);
  }
}

// ビットマップ送信要求
function ws_request_bitmap(fname, len) {
  if(websocket != null)
  {
    websocket.send('{"type":"bmp","name":"' + fname + '","size":' + len +'}');
  }
}

// ビットマップデータのバイナリ転送
function ws_request_bitmap_bin(data) {
  var len = data.length;
  
  // 2048単位で分割して送信する。
  // 大量に送ると受け取れない。
  var pos = 0;
  while(pos < len){
    var ary;
    if(len - pos > BIN_BLOCK_SIZE){
      ary = data.slice(pos, pos + BIN_BLOCK_SIZE);
    }else{
      ary = data.slice(pos);
    }

    if(websocket != null)
    {
      websocket.send(ary.buffer);
    }
    pos += ary.length;
  }

}
