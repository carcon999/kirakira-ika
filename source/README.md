ビルド＆書き込み手順の説明
===================

### 注意事項

現在資料整備中です。
少しづつ更新しています。全て公開されるまでしばらくお待ちください。
ライセンスは、[LICENSE.txt](https://raw.githubusercontent.com/carcon999/kirakira-ika/master/LICENSE.txt)をご確認ください。

# 概要

プログラムは、Arduino IDE 1.8.2でビルドし動作確認しています。  
ファームウェアを変更するには、ライブラリの準備や本体のジャンバー設定を変更する等の準備が必要になります。
手順をしっかり確認してから実施してください。ファームウェアの書き換えにはリスクを伴います。自己責任で実施してください。  

# ビルド環境の構築

## Arduino IDEをインストールする

IDEのインストール方法については、「Arduino IDE インストール」で検索することで見つけることができますので、
その手順に従ってインストールしてください。  当方では、Arduino IDE 1.8.2で検証しています。

インストール後、今回利用しているマイコンESP-WROOM-02(ESP8266)が利用できるように、ボードマネージャーにESP8266ボードをインストールする必要があります。
こちらも、ここでは詳細な説明をしませんので、「Arduino IDE ESP8266 インストール」で検索しインストールしてください。  

今回利用したバージョンは、「esp8266 by ESP8266 Commuity Version 2.3.0」になります。

![Image](https://raw.githubusercontent.com/carcon999/kirakira-ika/master/img/esp8266.png)

## ライブラリをインストールする

次に、利用するライブラリを２つ追加します。  
IDEの「スケッチ」→「ライブラリをインクルード」→「ライブラリを管理」を選択し、以下の２ライブラリをインストールします。

* ArduinoJson by Benolt Blanchon
* WebSockets by Markus Sattler 

![Image](https://raw.githubusercontent.com/carcon999/kirakira-ika/master/img/arduinojson.png)

![Image](https://raw.githubusercontent.com/carcon999/kirakira-ika/master/img/websocket.png)

## データ領域書き込みツールのインストール

データ領域（残像用の画像や設定HTMLファイル）に書き込むためには、ツール（[SPIFFSファイルシステムアップローダー](https://github.com/esp8266/arduino-esp8266fs-plugin)）
をインストールする必要があります。ツールをインストールすることで、Arduino IDEからデータ領域にデータを書き込むことができるようになります。

[SPIFFSファイルシステムアップローダー](https://github.com/esp8266/arduino-esp8266fs-plugin)をインストールしますが、インストーラは存在しないので、手動でファイルをコピーする必要があります。
幸い詳しく手順を説明しているブログがありますので、こちらを参考にしてください。

https://www.mgo-tec.com/spiffs-filesystem-uploader01-html

注意点としては、配置先の階層とフォルダ名に注意してください。下記の写真のような階層です。(画面はMacの場合です）

![Image](https://raw.githubusercontent.com/carcon999/kirakira-ika/master/img/esp8266jar.png)

配置後、IDEを起動（再起動）すると、ツールメニューの中に「ESP8266 Sketch Data Upload」表示されるようになります。

![Image](https://raw.githubusercontent.com/carcon999/kirakira-ika/master/img/data_upload.png)

以上でツール類のインストールは全て完了しました。

# 本体側の準備（基板の取り出しとジャンパー変更）

micro USBケーブルで本体とパソコンを接続し書き換えを行いますが、そのままの状態では書き換えは行えません。
基板上のジャンパー設定を変更した上で書き換えを実施する必要があります。そのためには、本体から基板を取り出す必要がありますが、
無理な力をかけて取り出すと故障の原因ともなりますのでご注意ください。

1. 本体の電源はOFFとなっていることを確認します。

2. ネジを２か所外し、背面カバーを外します。

3. 基板を取り出します。（チューブを5度程度折り曲げると取り出しやすいです。）
![Image](https://raw.githubusercontent.com/carcon999/kirakira-ika/master/img/kiraika_open.jpg)

4. 取り出した基板の前面に小型のジャンパーがあり、現在は、LED側となっているのをRX側にします。
![Image](https://raw.githubusercontent.com/carcon999/kirakira-ika/master/img/jumper.jpg)

では書き換えましょう！

# Flash領域の説明と全体の手順

### Flash領域の説明

ESP8266に内蔵されたFlashROMは、プログラム用に１Mバイト、データ用の３Mバイトとして定義されています。

現在は、プログラム領域は、USB（シリアルポート）経由で書き込みし、データ領域書き込みは、Wi-Fi経由のOTA（Over The Aire ）で書き込みを推奨とします。

※OTAで書き込みする場合は、プログラム領域にOTA用のプログラムが書き込まれている必要があります。

![Image](https://raw.githubusercontent.com/carcon999/kirakira-ika/master/img/flash_write.png)

もし、プログラム領域だけ書き換える必要がある場合は、STEP3へ進んでください。

### 全体の手順

少し複雑な手順になりますのでじっくりとすすめてください。

![Image](https://raw.githubusercontent.com/carcon999/kirakira-ika/master/img/fw_write.png)

* STEP1 OTAのファームをUSB経由で書き込む。

* STEP2 ネットワーク経由でDataを書き込む。

* STEP3 きらきら☆いかのファームをUSB経由で書き込む。

# STEP1 OTAファームをUSB経由で書き込む。

### OTAのファームを準備する。

OTAのファームは、スケッチ例に入っている「BasicOTA」を利用します。

![Image](https://raw.githubusercontent.com/carcon999/kirakira-ika/master/img/ota_basic.png)

ただし、一部コードを修正する必要があります。

BasicOTAを開き、ssidとpasswordの値をご自身のWi-Fi環境の値に変更します。

![Image](https://raw.githubusercontent.com/carcon999/kirakira-ika/master/img/ssid_pass.png)

変更したら、名前を付けて保存します。場所は、ご自身が分かり易い場所に保存してください。名前はなんでも構いませんが、BasicOTA_Kiraとでもしておきましょう。
この例では、デスクトップにBasicOTA_Kiraで保存しています。

![Image](https://raw.githubusercontent.com/carcon999/kirakira-ika/master/img/ota_save.png)

次に、書き込むデータをBasicOTA_Kiraのフォルダ直下にコピーします。データは、配布しているソースコードの一式中にdataフォルダがありますので、
そのフォルダ毎すべて直下にコピーします。（下記の写真のように）

![Image](https://raw.githubusercontent.com/carcon999/kirakira-ika/master/img/data_copy.png)

### OTAのファームを書き込む。

![Image](https://raw.githubusercontent.com/carcon999/kirakira-ika/master/img/step1.png)

0. IDEのプロジェクトは、BasicOTA_Kiraが開かれている状態とします。

1. 電源がOFF状態で、ジャンパーがRX側になっているのを確認します。

2. microUSBケーブルを接続し、PCと接続します。

3. 本体の電源を入れます。（LEDバーは点灯しません。）

4. IDEのシリアルポートを合わせます。

5. 書込みボタンを押して書き込みます。

6. 書込みが100%で完了したらOKです。

# STEP2 ネットワーク経由でDataを書き込む。

![Image](https://raw.githubusercontent.com/carcon999/kirakira-ika/master/img/step2.png)

0. STEP1の直後とします。

1. IDEを再起動(IDEを閉じて、もう一度BasicOTA_Kiraを開く)します。（これ重要）　本体はそのままでOKです。

2. 「ツール」→「シリアルポート」の中にネットワークのアドレスが表示されているはずなので選択します。

![Image](https://raw.githubusercontent.com/carcon999/kirakira-ika/master/img/net_write.png)

※もし、表示されてない場合は、本体の電源をOFF->ONして、IDEを再起動してみてください。

3. 「ツール」→「ESP8266 Sketch Data Upload」を選択します。画面に何も変化がありませんが、約30秒くらい待ちます。

4. 下記のような画面がでればデータ書込み成功です。「SPIFFS Image Uploaded」

![Image](https://raw.githubusercontent.com/carcon999/kirakira-ika/master/img/uploaded_ota.png)

# STEP3 きらきら☆いかのファームをUSB経由で書き込む。

![Image](https://raw.githubusercontent.com/carcon999/kirakira-ika/master/img/step3.png)

0. 電源がOFF状態で、ジャンパーがRX側になっているのを確認します。

1. microUSBケーブルを接続し、PCと接続します。

2. 本体の電源を入れます。（LEDバーは点灯しません。）

3. IDEで、きらきら☆いかのファームを開きます。（kiramini.inoを開きます。）

4. シリアルポートをUSBの仮想シリアルポートに合わせます。（ネットワークじゃない方）

5. IDEの書き込みボタンを押します。

6. もし失敗した場合は、本体の電源をOFF－＞ONし、再度書き込みボタンを押してください。

7. 繰り返し失敗する場合は、手順が間違っている可能性があります。再度手順（4～8）を確認してください。

8. 書き込みに成功（100%)したら、本体の電源を一度OFFにし、USBケーブルを外します。

9. 小型ジャンパーをLED側に戻し、本体の電源を入れて動作確認します。

10. 上手く動作したら電源をOFFにして、筐体に戻します。

# 最後に

新しいファームウェアがリリースされた場合は、各種設定ファイル（データ領域に配置）も新しくなっている場合があります。
基本的に、STEP1,STEP2,STEP3の順で実施してください。また、上手く起動しない場合も、データ領域を書き換え直して試してみてください。
