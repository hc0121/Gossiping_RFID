#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include "LED_HIGH.h"//設定ESP8266 LED亮
#include "LED_LOW.h"//設定ESP8266 LED熄滅
#include "WIFI_SET.h"//wifi帳號密碼儲存位置

constexpr uint8_t RST_PIN = D3;
constexpr uint8_t SS_PIN = D4;
MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
String tag1;
String tag;

//URL路徑或IP位置
//String serverName = "http://192.168.0.108:3000/api/gossiping/103/tags/";

unsigned long lastTime = 0;
// 如果要設定為10分鐘 timerDelay = 600000;
// 如果要設定為5秒鐘 timerDelay = 1000;
unsigned long timerDelay = 1000;

void setup() {
  Serial.begin(115200); 
  WIFI();
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  Serial.println("WiFi connected");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
  SPI.begin(); // 初始化SPI
  rfid.PCD_Init(); // 初始化MFRC522
  pinMode(D8, OUTPUT);
}

void loop() {
  if ( ! rfid.PICC_IsNewCardPresent())
    return;
  if (rfid.PICC_ReadCardSerial()) {
    for (byte i = 0; i < 4; i++) {
      tag += rfid.uid.uidByte[i];
    }

    Serial.println(tag);
    rfid.PICC_HaltA();}
  // 根據 timerDelay 對 HTTP POST 發送 request
  if ((millis() - lastTime) > timerDelay) {
    //檢查 WIFI 連接狀況
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
      //URL路徑或IP位置
      String serverName = "http://192.168.0.111:3000/api/gossiping/103/tags/";
      //102,163157131167
      //103,677094167
      if(tag == "677094167")
      {
        Serial.println("correct");
        serverName = serverName + tag ;
        LED_High();
      }
      else{
        Serial.println("error");
        serverName = serverName + tag;
        LED_Low();
      }
      String serverPath = serverName ;
      serverName=serverName+ "/";
      Serial.println(serverName);
      
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverPath.c_str());
      
      // 發送 HTTP GET request
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
    tag = "";
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
}
