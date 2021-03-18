/*
 * WebSocketClient.ino
 */
#include <Arduino.h>
#include <WiFi32.h> // From github.com/1amchris/WiFi32.h
#include <WebSocketsClient.h>
#include <FastLED.h>
#include <ArduinoJson.h>
#include "LedManager.h"

#define NAME "sword1"
#define WIFI_NAME "MakerSpace_2.4G" // TODO
#define WIFI_PWD "ntueesaad" // TODO
#define SERVER_IP "192.168.0.200" // TODO
#define SERVER_PORT 8080 // TODO

//StaticJsonDocument<90> doc;
int doc;
// DeserializationError error;
WebSocketsClient webSocket;
LedManager ledMgr;

void testEvent(String s) {
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, s);
  if (doc[0] == "play") {
    ledMgr.prepare_to_play(doc[1]["startTime"]);
    ledMgr.play();
  }
  else if (doc[0] == "pause") {
    ledMgr.pause();
  }
  else if (doc[0] == "stop") {
    ledMgr.pause_dark();
  }
  else if (doc[0] == "lightCurrentStatus"){
//    ledMgr.light_current_status(s);
  }
  else if (doc[0] == "uploadControl"){
      JsonArray s_json_array = doc[1];
      ledMgr.pause();
      ledMgr.parsing_json(s_json_array);
  }
}
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
    case WStype_DISCONNECTED:
      Serial.println("Websocket client disconnected");
      break;
    
    case WStype_CONNECTED:
      Serial.print("Websocket client connected to url: ");
      Serial.println((char *)payload);
      Serial.print("Send to server: ");
      char str[110];
      snprintf(str, 110, "[\"boardInfo\",{\"name\": \"%s\", \"OK\": true, \"msg\": \"success\", \"type\": \"dancer\"}]", NAME);
      Serial.println(str);
      webSocket.sendTXT(str);
      break;
     
    case WStype_TEXT:
      Serial.println("Websocket client get text: ");
      Serial.println((char *)payload);
      Serial.println();
      DynamicJsonDocument doc(10000); // TODO
      deserializeJson(doc, (char *)payload);
      String ss = doc[0];

      
      if (ss == "play")
      {
        ledMgr.prepare_to_play(doc[1]["startTime"]);
        ledMgr.play();
        char str[110];
        snprintf(str, 110, "[\"play\",{\"OK\": \"true\", \"msg\": \"Success\"}]");
        Serial.print("Send to server: ");
        Serial.println(str);
        webSocket.sendTXT(str);
      }
      
      else if (ss == "pause")
      {
        ledMgr.pause();
        snprintf(str, 110, "[\"pause\",{\"OK\": \"true\", \"msg\": \"Success\"}]");
        Serial.print("Send to server: ");
        Serial.println(str);
        webSocket.sendTXT(str);
      }
      
      else if (ss == "stop")
      {
        ledMgr.pause_dark();
        snprintf(str, 110, "[\"stop\",{\"OK\": \"true\", \"msg\": \"Success\"}]");
        Serial.print("Send to server: ");
        Serial.println(str);
        webSocket.sendTXT(str);
      }
      
      else if (ss == "uploadControl")
      {
        JsonArray s_json_array = doc[1];
        ledMgr.pause();
        if (ledMgr.parsing_json(s_json_array)){
          snprintf(str, 110, "[\"uploadControl\",{\"OK\": \"true\", \"msg\": \"Success\"}]");
          Serial.print("Send to server: ");
          Serial.println(str);
          webSocket.sendTXT(str);
        }
        else {
          snprintf(str, 110, "[\"uploadControl\",{\"OK\": \"false\", \"msg\": \"Failed\"}]");
          Serial.print("Send to server: ");
          Serial.println(str);
          webSocket.sendTXT(str);
        }
      }
      
      else if (ss == "lightCurrentStatus")
      {
        DynamicJsonDocument s_json = doc[1];
        serializeJson(s_json, Serial);
        ledMgr.light_current_status(s_json);
        snprintf(str, 110, "[\"lightCurrentStatus\",{\"OK\": \"true\", \"msg\": \"Success\"}]");
        Serial.print("Send to server: ");
        Serial.println(str);
        webSocket.sendTXT(str);
      }
    break;
  }
}


void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  delay(100);
  WiFi.begin(WIFI_NAME, WIFI_PWD);
  if (testWifi()) {
    Serial.println("WiFi connected OK");
    Serial.print("Local IP: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println("WiFi connected NG");
  }
  
  webSocket.begin(SERVER_IP, SERVER_PORT, "/"); // server address, port, URL
  
  webSocket.onEvent(webSocketEvent); // event handler

  webSocket.setReconnectInterval(5000); // try ever 5000 again if connection has failed
  // heartbeat (optional): ping every 15000 ms; expect pong within 3000 ms; consider disconnected if pong is not received 2 times
  // webSocket.enableHeartbeat(15000, 3000, 2);

  pinMode(13, INPUT); // Initialize the LED_BUILTIN pin as an output
  delay(10);
  // char str[100];
  // snprintf(str, 100, "{\"hostname\": \"%s\"}", NAME);
  // Serial.println(str);
  // webSocket.sendTXT(str);
  ledMgr.init();
  delay(1000);
  Serial.println("ready");

//  String char_s = "[[{\"start\": 0,\"fade\": false, \"status\": {\"led_sword\": {\"src\": \"green\", \"alpha\": 1},}},\
//        {\"start\": 3000,\"fade\": false, \"status\": {\"led_sword\": {\"src\": \"blue\", \"alpha\": 1},}},\
//        {\"start\": 6000,\"fade\": false, \"status\": {\"led_sword\": {\"src\": \"dark\", \"alpha\": 0},}},\
//        {\"start\": 90000,\"fade\": false, \"status\": {\"led_sword\": {\"src\": \"red\", \"alpha\": 1},}}\
//        ]]";
//    DynamicJsonDocument s_json(5000);
//    deserializeJson(s_json, char_s);
//    Serial.println("end of parsing");
//    serializeJson(s_json, Serial);
//    JsonArray s = s_json[0];
//    ledMgr.parsing_json(s);
}

bool testWifi(void) {
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED) { return true; }
    delay(500);
    Serial.print(WiFi.status());  
    c++;
  }
  return false;
}

void loop()
{
//  webSocket.loop();
  ledMgr.loop();
       
  
  if (Serial.available() > 0) {
    String s = Serial.readString();
    char* char_s = "[{\"start\": 0,\"fade\": false, \"status\": {\"led_sword\": {\"src\": \"green\", \"alpha\": 1},}},\
    {\"start\": 3000,\"fade\": false, \"status\": {\"led_sword\": {\"src\": \"blue\", \"alpha\": 1},}},\
    {\"start\": 6000,\"fade\": false, \"status\": {\"led_sword\": {\"src\": \"dark\", \"alpha\": 0},}},\
    {\"start\": 90000,\"fade\": false, \"status\": {\"led_sword\": {\"src\": \"red\", \"alpha\": 1},}}\
    ]";
    testEvent(s);
//    delay(3000);
//    testEvent(char* "[\"play\"]");
  }
}
