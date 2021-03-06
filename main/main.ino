#include <Arduino.h>
#include <WiFi32.h> // From github.com/1amchris/WiFi32.h
#include <WebSocketsClient.h>
#include <FastLED.h>
#include <ArduinoJson.h>
#include "LedManager.h"

// Sword name, wifi router and server info
#define NAME "sword10"
#define WIFI_NAME "MakerSpace_2.4G"
#define WIFI_PWD "ntueesaad"
#define SERVER_IP "192.168.0.200"
#define SERVER_PORT 8080

int doc;
WebSocketsClient webSocket;
LedManager ledMgr;

void testEvent(String s) {
  /*
  This function is used to test LedManager using keyboard.
  Useful when the server hasn't been set yet.
  */
  if (s[2] == 'u'){
    Serial.println("upload");
    ledMgr.pause();
    ledMgr.parsing_json(s);
  }

  else {
    DynamicJsonDocument doc(300);
    deserializeJson(doc, s);
    serializeJson(doc, Serial);
    Serial.println();
    if (doc[0] == "play") {
      ledMgr.prepare_to_play(doc[1]["startTime"], doc[1]["delay"]);
      ledMgr.play();
    }
    else if (doc[0] == "pause") {
      ledMgr.pause();
    }
    else if (doc[0] == "stop") {
      ledMgr.pause_dark();
    }
    else if (doc[0] == "lightCurrentStatus"){
      ledMgr.light_current_status(doc[1]);
    }
  }
}


void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  /*
  This function is used to get commands from the server and give responses back.
  */
  switch (type) {
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
      /*
      commands: uploadcontrol, play, pause, stop, lightCurrentStatus
      */
      if (*(((char *)payload) + 2) == 'u'){
        ledMgr.pause();
        if (ledMgr.parsing_json((char *)payload)) {
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
      else {
        DynamicJsonDocument doc(500); 
        deserializeJson(doc, (char *)payload);
        String ss = doc[0];
        if (ss == "play")
        {
          ledMgr.prepare_to_play(doc[1]["startTime"], doc[1]["delay"]);
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
        
        else if (ss == "lightCurrentStatus")
        {
          StaticJsonDocument<300> s_json = doc[1];
          serializeJson(s_json, Serial);
          ledMgr.light_current_status(s_json);
          snprintf(str, 110, "[\"lightCurrentStatus\",{\"OK\": \"true\", \"msg\": \"Success\"}]");
          Serial.print("Send to server: ");
          Serial.println(str);
          webSocket.sendTXT(str);
        }
      }
    break;
  }
}


void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  delay(100);

  // set up wifi
  WiFi.begin(WIFI_NAME, WIFI_PWD);
  if (testWifi()) {
    Serial.println("WiFi connected OK");
    Serial.print("Local IP: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println("WiFi connected NG");
  }
  
  // set up webSocket
  webSocket.begin(SERVER_IP, SERVER_PORT, "/"); // server address, port, URL
  webSocket.onEvent(webSocketEvent); // event handler
  webSocket.setReconnectInterval(5000); // try ever 5000 again if connection has failed
  // heartbeat (optional): ping every 15000 ms; expect pong within 3000 ms; consider disconnected if pong is not received 2 times
  // webSocket.enableHeartbeat(15000, 3000, 2);

  // set up led manager
  pinMode(13, INPUT); // Initialize the LED_BUILTIN pin as an output
  delay(10);
  ledMgr.init();
  delay(1000);

  Serial.println("ready");
  // at the beginning, all led dark, timeline to 0
  ledMgr.pause_dark();
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


void loop() {
  webSocket.loop();
  ledMgr.loop();
  
  // test
  // if (Serial.available() > 0) {
  //   String s = Serial.readString();
  //   testEvent(s);
  // }
}
