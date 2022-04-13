# esp32-oscrcv-dmxsend

<img src="https://github.com/mathrax-s/esp32-oscrcv-dmxsend/blob/main/image/device_1.jpg" width=40%></img>
<img src="https://github.com/mathrax-s/esp32-oscrcv-dmxsend/blob/main/image/pd.png" width=40% ></img>

WiFi経由でOSC(Open Sound Control)信号を受信してDMX信号を送信するデバイスと、そのデバイスにOSC送信するPureDataファイルのセットです。

<br>ESP32ボード
<br>https://akizukidenshi.com/catalog/g/gK-16108/
<br>&nbsp;&nbsp;17pin　...　TX
<br>&nbsp;&nbsp;16pin　...　RX
<br>&nbsp;&nbsp;21pin　...　EN
<br>&nbsp;&nbsp;13pin　...　LED
<br>
<br>USBシリアルケーブル（プログラム書き換え時のみ使用）
<br>https://akizukidenshi.com/catalog/g/gM-05841/
<br>
<br>MAX3485(RS485ドライバIC）
<br>https://www.maximintegrated.com/jp/products/interface/transceivers/MAX3485.html
<br>
<br>XLR3 レセプタクルコネクタ
<br>https://www.soundhouse.co.jp/products/detail/item/45854/
## ArduinoOTA(WiFi経由で書き換え）
WiFiのSSIDやパスワードや固定IPなどをプログラム内に書いた場合、うっかり間違った情報を書き込むとWiFiがつながらなります。吊り作品に使用することを想定しているので、それを避けたく、ESP32のEEPROMにあらかじめ書き込んだ情報を読み込むことにしています。EEPROMに書き込むプログラムは、<a href="https://github.com/mathrax-s/esp32-oscrcv-dmxsend/blob/main/Arduino/esp32_oscrcv_dmxsend/EEPROM.ino">EEPROM.ino</a>にあります。


## OSC(Open Sound Control)受信
### OSC library
https://github.com/CNMAT/OSC
<br>いろいろなOSCライブラリがありますが、本家CNMATのライブラリにしました。

## DMX送信
### esp_dmx library
https://www.arduino.cc/reference/en/libraries/esp_dmx/
<br>いろいろなDMXライブラリがありますが、ESP用のライブラリにしました。
<br>OSC受信した信号をすぐにDMX送信すると、光がそろわない場合があったので、複数チャンネルのデータを送信する機能と、DMX送信するチャンネル番号を指定する機能に分けました。PureDataのサンプルでは、赤青緑のLEDライトを想定していて、ポート8888でCh1=赤・Ch2=緑・Ch3=青のデータを送信し、ポート8889で「loadbang」で「3」を送信しています。デバイスはCh1・Ch2・Ch3のデータを受信して、Ch3を受信したときはじめてDMX信号を送信します。

## FreeRTOS
ライブラリはありませんが、メモしておきます。複数のタスクとArduinoOTAを併用する場合、それぞれのタスクに「ArduinoOTA.handle();」を記述することで書き換えができました。
