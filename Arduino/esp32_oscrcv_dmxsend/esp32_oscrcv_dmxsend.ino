// EEPROMでIP設定
//  本番プログラムにはIP書かない
//  あらかじめEEPROMに書き込んでおいて、本番プログラムはEEPROM読むだけに
//
// OSC受信
//  OSC library
//  https://github.com/CNMAT/OSC
//
// DMX送信
//  esp_dmx library
//  https://www.arduino.cc/reference/en/libraries/esp_dmx/
//
// FreeRTOS
//  それぞれのTaskに、ArduinoOTA.handle();

#include "EEPROM.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// OSC
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

WiFiUDP Udp;
const unsigned int localPort = 8888;  // local port
OSCErrorCode error;
int ledChannel = 0;
int ledValue = 0;

// WiFi
int address = 0;
String ssid = "";
String password = "";
String local_ip_str = "";
String gateway_ip_str = "";
String subnet_ip_str = "";

IPAddress _local_ip;
IPAddress _gateway;
IPAddress _subnet;

// DMX
#include <Arduino.h>
#include <esp_dmx.h>
int transmitPin = 17;
int receivePin = 16;
int enablePin = 21;
dmx_port_t dmxPort = 2;
byte data[DMX_MAX_PACKET_SIZE];
uint8_t send_done = 0;

// FreeRTOS
#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

void TaskOsc( void *pvParameters );
void TaskDmx( void *pvParameters );


// --------------------------------------------
// SETUP
void setup() {
  Serial.begin(115200);
  ledcSetup(0, 4000, 8);
  ledcAttachPin(13, 0);
  for (int i = 0; i < 10; i++) {
    ledcWrite(0, 10);
    delay(50);
    ledcWrite(0, 0);
    delay(50);
  }

  dmx_config_t dmxConfig = DMX_DEFAULT_CONFIG;
  dmx_param_config(dmxPort, &dmxConfig);
  dmx_set_pin(dmxPort, transmitPin, receivePin, enablePin);
  int queueSize = 0;
  int interruptPriority = 1;
  dmx_driver_install(dmxPort, DMX_MAX_PACKET_SIZE, queueSize, NULL, interruptPriority);
  dmx_set_mode(dmxPort, DMX_MODE_TX);
  data[0] = 0;

  if (!EEPROM.begin(1000)) {
    Serial.println("Failed to initialise EEPROM");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }

  //  EEPROM_write();
  EEPROM_read();
  _local_ip.fromString(local_ip_str);
  _gateway.fromString(gateway_ip_str);
  _subnet.fromString(subnet_ip_str);

  WiFi.mode(WIFI_STA);
  if (!WiFi.config(_local_ip, _gateway, _subnet)) {
    Serial.println("STA Failed to configure");
  }

  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("\nAuth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("\nBegin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("\nConnect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("\nReceive Failed");
    else if (error == OTA_END_ERROR) Serial.println("\nEnd Failed");
  });

  ArduinoOTA.begin();

  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(localPort);

  xTaskCreateUniversal(
    TaskOsc
    ,  "TaskOsc"
    ,  8192
    ,  NULL
    ,  2
    ,  NULL
    ,  ARDUINO_RUNNING_CORE);

  xTaskCreateUniversal(
    TaskDmx
    ,  "TaskDmx"
    ,  8192
    ,  NULL
    ,  1
    ,  NULL
    ,  ARDUINO_RUNNING_CORE);
}


// --------------------------------------------
// OSC受信の処理
void led(OSCMessage &msg) {
  ledChannel = (int)(msg.getFloat(0)) % 513;
  ledValue = (int)(msg.getFloat(1)) % 256;
  ledcWrite(0, ledValue);
  Serial.print("/led: ");
  Serial.print(ledChannel);
  Serial.print(" , ");
  Serial.println(ledValue);
  if (ledChannel < DMX_MAX_PACKET_SIZE) {
    data[ledChannel] = ledValue;
    dmx_write_packet(dmxPort, data, DMX_MAX_PACKET_SIZE);
  }
}
void send_dmx(OSCMessage &msg) {
  send_done = (int)(msg.getFloat(0));
  Serial.print("/send_done: ");
  Serial.println(send_done);
}

// --------------------------------------------
// TASK
// --------------------------------------------
// DMX送信タスク
void TaskDmx(void *pvParameters) {
  (void) pvParameters;
  while (1) {
    ArduinoOTA.handle();
    if (ledChannel == send_done) {
      ledChannel = 0;
      dmx_tx_packet(dmxPort);
      dmx_wait_tx_done(dmxPort, DMX_TX_PACKET_TOUT_TICK);
    }
    vTaskDelay(1);
  }
}

// OSC受信タスク
void TaskOsc(void *pvParameters) {
  (void) pvParameters;
  while (1)
  {
    ArduinoOTA.handle();
    OSCMessage msg;
    int size = Udp.parsePacket();

    if (size > 0) {
      while (size--) {
        msg.fill(Udp.read());
      }
      if (!msg.hasError()) {
        msg.dispatch("/led", led);
        msg.dispatch("/send_ch", send_dmx);
      } else {
        error = msg.getError();
        Serial.print("error: ");
        Serial.println(error);
      }
    }
    vTaskDelay(1);
  }
}

// --------------------------------------------
// LOOP
void loop() {
}
