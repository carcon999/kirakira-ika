/*
 * kira.js
 */


//const HTTP_LOCAL= "http://192.168.10.1";   // local debug
const HTTP_LOCAL= "";   // Release


// 設定情報更新
function update_display_system_setting() {

  // 項目の一覧取得
  requestAjax(HTTP_LOCAL + "/set", function(response){
  
    // LED数の選択更新
    var ledlength = document.getElementById('ledlength');
    var options = ledlength.options;
    for(var i = 0 ; i<options.length ; i++){
      if(options[i].value == response.length){
        ledlength.selectedIndex = i;
        break;
      }
    }
    
    // 明るさの選択更新
    var ledbright = document.getElementById('ledbright');
    var options = ledbright.options;
    for(var i = 0 ; i<options.length ; i++){
      if(options[i].value == response.bright){
        ledbright.selectedIndex = i;
        break;
      }
    }

    // Wi-Fiの設定更新
    var en_wifi = document.getElementById('wifiset');
    var options = en_wifi.options;
    for(var i = 0 ; i<options.length ; i++){
      if(options[i].value == response.wifi){
        en_wifi.selectedIndex = i;
        break;
      }
    }
  });
};

// システム設定の変更を要求する。
function request_system_setting_change() {
  // 選択中の項目を取得
  // LED数の選択項目
  var len_option = document.getElementById('ledlength');
  // 明るさの選択項目
  var br_option = document.getElementById('ledbright');
  // WiFiの選択項目
  var wifi_option = document.getElementById('wifiset');

  // パラメータ生成
  var prm = len_option.options[len_option.selectedIndex].value  + "," + br_option.options[br_option.selectedIndex].value + "," + wifi_option.options[wifi_option.selectedIndex].value;

  // 設定要求
  requestAjax(HTTP_LOCAL + "/set?prm=" + prm ,
    function(response){
    // 成功表示
    }
    ,
    function(response){
    // 失敗表示
      alert(response[0].result);
    }
  );
}

// ファイルアップロード
function upload_file()
{
  // FormData オブジェクトを作成
  var formData = new FormData();
  var tmp = $('input[type=file]')[0].files[0];
  formData.append('file', tmp);

  // POSTでアップロード
  $.ajax({
    type        : "POST",
    url         : HTTP_LOCAL + "/upload",
    data        : formData,
    cache       : false,
    contentType : false,
    processData : false,
    dataType    : "json",
    timeout     : 10000,
  })
  .done(function(data, textStatus, jqXHR){
    // alert(data[0].result);
    // リスト更新
    update_list_display();
  })
  .fail(function(jqXHR, textStatus, errorThrown){
    alert("fail");
  });
}

// ファイルリスト更新
function update_list_display() {
  // 既存のリストはクリア
  $('select#select option').remove();

  // リストの一覧取得
  requestAjax(HTTP_LOCAL + "/list", function(response){
    var select = document.getElementById('select');
    
    // ファイルリストを取り出しリスト表示へ追加
    for(key in response){
      var option = document.createElement('option');
      option.setAttribute('value', response[key].name);
      option.innerHTML = response[key].name + " : " + response[key].size + " byte";
      select.appendChild(option);
    }
    
    // ファイル削除ボタンの活性制御
    activation_delete_button();
  });
};

// ファイル選択情報表示
$(document).on('change', ':file', function()
{
  var input = $(this),
  numFiles = input.get(0).files ? input.get(0).files.length : 1,
  label = input.val().replace(/\\/g, '/').replace(/.*\//, '');
  input.parent().parent().next(':text').val(label);

  var files = !!this.files ? this.files : [];
  if (!files.length || !window.FileReader) {
    
    // クリア
    $("#fileName").text("Name: n/a");
    $("#fileSize").text("Size: n/a");
    $("#fileType").text("Type: n/a");
    $("#imgWidth").text("Width: n/a" );
    $("#imgHeight").text("Height: n/a" );
    $("#imgBitcount").text("Bitcount: n/a" );
    $("#imgInfo").text("user cancel.");
    var preview = input.parent().parent().parent().prev('.imagePreview');
    preview.css("background-image", "none");
    document.getElementById("uploadBtn").disabled = "true";

    return; // no file selected, or no FileReader support
  }
  
  check_file(files[0]);
  
  if (/^image/.test( files[0].type)){ // only image file
      var reader = new FileReader(); // instance of the FileReader
      reader.readAsDataURL(files[0]); // read the local file
      reader.onloadend = function(){ // set image data as background of div
          var preview = input.parent().parent().parent().prev('.imagePreview');
          preview.css("background-image", "url("+this.result+")");
      }
  }
});

// アップロードするファイルをチェックする
function check_file(file) {
  if (file) {
    $("#fileName").text("Name: " + file.name);
    $("#fileSize").text("Size: " + file.size);
    $("#fileType").text("Type: " + file.type);
    
    var fr = new FileReader();
    
    fr.onload = function(event){
      var data = event.target.result;
      var buf = new Uint8Array(data);
      var length = buf.byteLength;
      var width = 0;
      var height = 0;
      var bitcount = 0;
      
      if(length > 54){
        // BMヘッダーチェック+画像のサイズ取得
        if (buf[0] == 0x42 && buf[1] == 0x4D) {
          // 画像サイズ
          width = buf[18] + (buf[19] << 8) + (buf[20] << 16) + (buf[21] << 24);
          height = buf[22] + (buf[23] << 8) + (buf[24] << 16) + (buf[25] << 24);
          // ビットカウント
          bitcount = buf[28] + (buf[29] << 8);
        }
      }
      
      if(bitcount == 24){
        $("#imgWidth").text("Width: " + width );
        $("#imgHeight").text("Height: " + height );
        $("#imgBitcount").text("Bitcount: " + bitcount );
        $("#imgInfo").text("");
        document.getElementById("uploadBtn").disabled = "";
      }else{
        $("#imgWidth").text("Width: n/a" );
        $("#imgHeight").text("Height: n/a" );
        $("#imgBitcount").text("Bitcount: n/a" );
        $("#imgInfo").text("24bit bitmap file only.");
        document.getElementById("uploadBtn").disabled = "true";
      }
    };

    fr.readAsArrayBuffer(file);
  }
}

// ファイル削除処理
function remove_files() {
  // 選択中のリストを取得
  // オプションのリスト
  var options = document.getElementById('select').options;
  var names = "";

  // 選択されている値をカンマ区切りで複数を連結する
  var count = 0;
  for(var i = 0 ; i<options.length ; i++){
    if( options[ i ].selected ){
      if(names.length != 0){
        names += ',';
      }
      names += options[ i ].value;
      count++;
    }

    // 最大10単位なので区切る
    if(count >= 10 || i == (options.length - 1)){
      requestAjax(HTTP_LOCAL + "/del?names=" + names ,
        function(response){
        // 成功表示
        }
        ,
        function(response){
        // 失敗表示
          alert(response[0].result);
        }
      );
      names = "";
      count = 0;
    }
  }

  // リスト更新
  update_list_display();
};

// ファイル選択変更に伴う削除ボタンの活性制御
function activation_delete_button() {
  var select = document.getElementById('select');
  var index = select.selectedIndex;
  if(index >= 0){
    document.getElementById("deleteBtn").disabled = "";
  }else{
    document.getElementById("deleteBtn").disabled = "true";
  }
}

// Ajaxリクエスト共通処理
function requestAjax(endpoint, callback, errcallback) {
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function(){
    if (this.readyState==4 && this.status==200) {
      if(callback != null){
        callback(this.response);
      }
    }else if(this.readyState >= 400) {
      if(errcallback != null){
        errcallback(this.response);
      }
    }
  };
  xhr.responseType = 'json';
  xhr.open('GET',endpoint,true);
  xhr.send();
};
