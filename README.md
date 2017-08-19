きらきら☆いか
===================

### 概要

『きらきら☆いか』は、残像表示が得意なLEDバーになります。

![Image](https://raw.githubusercontent.com/carcon999/kirakira-ika/master/img/kiraika.jpg)

簡単な紹介動画はこちらです。
https://youtu.be/-Ymk3oiKI8k

### 取扱い説明書
本体の説明書になります。
[guidebook](https://github.com/carcon999/kirakira-ika/tree/master/guidebook)フォルダにPDFファイルを公開しています。  

### 回路図
ハードウェアの回路図を公開しました。
ESP-WROOM-02(ESP8266)を利用しています。LEDは3535サイズの[SK6812MINI](https://cdn-shop.adafruit.com/product-files/2686/SK6812MINI_REV.01-1-2.pdf)を利用しています。表裏合わせると最大64個のLEDを搭載できる設計です。  
[schematic](https://github.com/carcon999/kirakira-ika/tree/master/schematic)フォルダにPDFファイルを公開しています。 

### ガーバー
基板製造用のgerberを公開しました。
[gerber](https://github.com/carcon999/kirakira-ika/tree/master/gerber)フォルダに一式公開しています。基板を３面付けしたデータを公開しています。表示するには、別途Gerber Viewerが必要です。

### プログラム（本体ソースコード）
Arduino IDEでビルド可能な、本体プログラムを公開しています。
[source](https://github.com/carcon999/kirakira-ika/tree/master/source)フォルダに一式公開しています。
ビルド方法は、その中にある[README.md](https://github.com/carcon999/kirakira-ika/tree/master/source/README.md)を参照してください。

### 筐体モデルデータ
モデリングした本体の筐体データを公開しました。  
[3dmodel](https://github.com/carcon999/kirakira-ika/tree/master/3dmodel)フォルダにSTLファイルを公開しています。  
３Dプリンタで印刷することができます。  

### きら☆いか リモコン
アンドロイド端末で利用できるWi-FiリモコンはGoogle Playから[ダウンロード](https://play.google.com/store/apps/details?id=jp.gr.java_conf.approid.kiraika_remo)することができます。  

![Image](https://raw.githubusercontent.com/carcon999/kirakira-ika/master/img/kiraika-icon.png)  

### リモコン用ソースコード  
リモートコントロールは、 **WebSocket**が利用できるため、ブラウザ経由で制御することができます。[remotecontrol](https://github.com/carcon999/kirakira-ika/tree/master/remotecontrol)フォルダに一式公開しております。動作確認は、Chromeでのみ確認しております。
PCのWi-Fi接続先をきらきら☆いかのアクセスポイントにすることで、パソコンからコントロールすることができます。  
※こちらは、本体用のプログラムではありません。リモートコントロール用です。

