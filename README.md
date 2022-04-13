# esp32-oscrcv-dmxsend

<img src="https://github.com/mathrax-s/esp32-oscrcv-dmxsend/blob/main/image/device_1.jpg" width=40%></img>
<img src="https://github.com/mathrax-s/esp32-oscrcv-dmxsend/blob/main/image/pd.png" width=40% ></img>

WiFi経由でOSC(Open Sound Control)信号を受信し、DMX信号を送信するデバイスと、PureDataのセットです。

<br>ESP32ボード
<br>https://akizukidenshi.com/catalog/g/gK-16108/
<br>USBシリアルケーブル（プログラム書き換え時のみ使用）
<br>https://akizukidenshi.com/catalog/g/gM-05841/
<br>
<br>RS485ドライバ（3.3V MAX3485）
<br>https://www.maximintegrated.com/jp/products/interface/transceivers/MAX3485.html
<br>
<br>
## ArduinoOTAで、WiFiで書き換え
<br>WiFiのSSIDやパスワードや固定IPなどをプログラム内に書いた場合、うっかり間違った情報を書き込むとWiFiがつながらなります。吊り作品に使用することを想定しているので、それを避けたく、ESP32のEEPROMにあらかじめ書き込んだ情報を読み込むことにしています。EEPROMに書き込むプログラムは、<a href="https://github.com/mathrax-s/esp32-oscrcv-dmxsend/blob/main/Arduino/esp32_oscrcv_dmxsend/EEPROM.ino">EEPROM.ino</a>にあります。


## OSC(Open Sound Control)受信
<br>OSC library
<br>https://github.com/CNMAT/OSC
<br>いろいろなOSCライブラリがありますが、本家CNMATのライブラリにしました。

## DMX送信
<br>esp_dmx library
<br>https://www.arduino.cc/reference/en/libraries/esp_dmx/
<br>いろいろなDMXライブラリがありますが、ESP用のライブラリにしました。

## FreeRTOS
<br>ライブラリはありませんが、メモしておきます。複数のタスクとArduinoOTAを併用する場合、それぞれのタスクに「ArduinoOTA.handle();」を記述することで書き換えができました。
