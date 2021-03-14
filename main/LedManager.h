#ifndef LED_MANAGER
#define LED_MANAGER
#include <FastLED.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <typeinfo>

#define NAME "sword1"

#define KNIFE_NUM 59
#define SHIELD_NUM 18  
#define HANDLE_NUM 16
#define KNIFE_PIN 16  
#define SHIELD_PIN 17  
#define HANDLE_PIN 5
#define KNIFE_BRIGHTNESS 32/255 
#define SHIELD_BRIGHTNESS 10/255
#define HANDLE_BRIGHTNESS 10/255


class LedManager{
  public:
//    LedManager(): starting_time_abs(0), playing_time(0), playing(false) {
//    }
//    ~LedManager() {}

    void init() {
      FastLED.addLeds<WS2812B, KNIFE_PIN, GRB>(KNIFE_leds, KNIFE_NUM);  //設定串列全彩LED參數
      FastLED.addLeds<WS2812B, SHIELD_PIN, GRB>(SHIELD_leds, SHIELD_NUM);  //設定串列全彩LED參數
      FastLED.addLeds<WS2812B, HANDLE_PIN, GRB>(HANDLE_leds, HANDLE_NUM);  //設定串列全彩LED參數
      FastLED.setBrightness(32);
      
      error = deserializeJson(pic_json, pic_data); // debug
      if(error) {
        Serial.println("Parsing Error: Pic_data");
        Serial.println(error.c_str());
        delay(5000);
      }
      starting_time_abs = 0;
      playing_time = 0;
      playing = false;
      frame_idx = 0;
    }

    bool parsing_json(unsigned char* data) {
      error = deserializeJson(tmp_json, data); 
      led_json = tmp_json[1];
      if(error) {
        Serial.println("Parsing Error: led_json");
        Serial.println(error.c_str());
        return false;
      }
      return true;
    }

    void prepare_to_play(unsigned long s_time = 0) {
      Serial.println("play from");
      Serial.println(s_time);
      Serial.println();
      playing_time = s_time;
      starting_time = s_time;
      set_frame_idx();
    }
  
    void play() {
      Serial.println("Start Playing");
      starting_time_abs = millis();
      playing = true;
    }

    void pause() {
      playing = false;
    }

    void pause_dark() {
      playing = false;
      all_dark();
    }
    void show_frame() {
      const String src_sword = led_json[frame_idx]["status"]["LED_SWORD"]["src"]; // A name, which responds to a picture
      double alpha_sword = led_json[frame_idx]["status"]["LED_SWORD"]["alpha"];
      JsonArray pic_sword = pic_json[src_sword]; // The picture at this time, every led is represented as one string, eg: "0x00FF00"
      const String src_guard = led_json[frame_idx]["status"]["LED_GUARD"]["src"];
      double alpha_guard = led_json[frame_idx]["status"]["LED_GUARD"]["alpha"];
      JsonArray pic_guard = pic_json[src_guard];
      const String src_handle = led_json[frame_idx]["status"]["LED_HANDLE"]["src"];
      double alpha_handle = led_json[frame_idx]["status"]["LED_HANDLE"]["alpha"];
      JsonArray pic_handle = pic_json[src_handle];
      
      for(int i = 0; i < KNIFE_NUM; i++) {
        KNIFE_leds[i] = strtol(pic_sword[i], NULL, 0);
      }

      for(int i = 0; i < SHIELD_NUM; i++) {
        SHIELD_leds[i] = strtol(pic_guard[i], NULL, 0);
      }

      for(int i = 0; i < HANDLE_NUM; i++) {
        HANDLE_leds[i] = strtol(pic_handle[i], NULL, 0);
      }
      

      Serial.print(millis());
      Serial.print(src_sword);
      Serial.print(src_guard);
      Serial.print(src_handle);
      Serial.print("  ");
      Serial.println(playing_time);
      FastLED.show();
    }

    void light_current_status(unsigned char* data) {
      error = deserializeJson(tmp_json, data);
      if(error) {
        Serial.println("Parsing Error: status");
        Serial.println(error.c_str());
      }
      status = tmp_json[1];
      const String src_sword = status["led_sword"]["src"]; 
      double alpha_sword = status["led_sword"]["alpha"];
      const String src_guard = status["led_guard"]["src"]; 
      double alpha_guard = status["led_guard"]["alpha"];
      const String src_handle = status["led_handle"]["src"]; 
      double alpha_handle = status["led_handle"]["alpha"];
      JsonArray pic_sword = pic_json[src_sword]; // The picture at this time, every led is represented as one string, eg: "0x00FF00"
      serializeJson(pic_sword, Serial);
      Serial.println();
      JsonArray pic_guard = pic_json[src_guard]; // The picture at this time, every led is represented as one string, eg: "0x00FF00"
      serializeJson(pic_guard, Serial);
      Serial.println();
      JsonArray pic_handle = pic_json[src_handle]; // The picture at this time, every led is represented as one string, eg: "0x00FF00"
      serializeJson(pic_handle, Serial);
      Serial.println();
      
      for(int i = 0; i < KNIFE_NUM; i++) {
        KNIFE_leds[i] = strtol(pic_sword[i], NULL, 0);
//          KNIFE_leds[i] = CRGB(255, 255, 255);
          
      }

      for(int i = 0; i < SHIELD_NUM; i++) {
        SHIELD_leds[i] = strtol(pic_guard[i], NULL, 0);
//          SHIELD_leds[i] = CRGB(255, 255, 255);
      }

      for(int i = 0; i < HANDLE_NUM; i++) {
        HANDLE_leds[i] = strtol(pic_handle[i], NULL, 0);
//          HANDLE_leds[i] = CRGB(255, 255, 255);
      }
      
      FastLED.show();
      
    }

    void set_frame_idx() {
      // To set the frame index, according to the playing time and the timeline
      while(!frame_end() && playing_time >= led_json[frame_idx + 1]["start"]) {
        frame_idx += 1;
      }
      while(frame_idx != 0 && playing_time < led_json[frame_idx]["start"]) {
        frame_idx -= 1;
      }
    }

    bool frame_end() {
      if(frame_idx == led_json.size() - 1)
        return true;
      return false;
    }

    void all_dark() {
      for(int i = 0; i < KNIFE_NUM; i++) {
        KNIFE_leds[i] = CRGB::Black;
      }
      for(int i = 0; i < SHIELD_NUM; i++) {
        SHIELD_leds[i] = CRGB::Black;
      }
      for(int i = 0; i < HANDLE_NUM; i++) {
        HANDLE_leds[i] = CRGB::Black;
      }
    }

    void loop() {
      if(playing) {
        playing_time = millis() - starting_time_abs + starting_time;
        if(frame_idx == led_json.size() - 1) {
            // The end of the show
            playing = false;
        }
        else if(playing_time > led_json[frame_idx+1]["start"]){
          frame_idx += 1;
          Serial.println("showing frame");
          show_frame();
          
        }
      }
    }
    


  private:
    // StaticJsonDocument<20000> led_json; 
    JsonArray led_json;
    /* led timeline
    {
      "type": "uploadControl",
      "data":
        [
          {
            "start": 0,
              "status": {
                "src": "all_light",
                "alpha": "0.5"
              }
            },
          {
            "start": 3000,
            "status": {
              "src": "black",
              "alpha": "0.5"
            }
          }
        ]
     }
     */
    StaticJsonDocument<10000> pic_json; 
    /* led frame
     * {
     * "red": [],
     * "black": {}
     * }
     */
    StaticJsonDocument<1000> status;
    StaticJsonDocument<20000> tmp_json;

    const char* uploadControl_json = "[\"uploadControl\",[{\"start\":0,\"fade\":false,\"status\":{\"LED_HANDLE\":{\"src\":\"bl_handle\",\"alpha\":0},\"LED_GUARD\":{\"src\":\"bl_guard\",\"alpha\":0},\"LED_SWORD\":{\"src\":\"bl_sword\",\"alpha\":0}}}]]";

    
    const char* pic_data = "{\
      \"bl_sword\":[\"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\"],\
      \"red_sword\":[\"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\", \"0xFF0000\",\"0xFF0000\"], \
      \"green_sword\":[\"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\"],\
      \"blue_sword\":[\"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\"], \
      \"red_guard\":[\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\"], \
      \"green_guard\":[\"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\"], \
      \"blue_guard\":[\"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\"], \
      \"bl_guard\":[\"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\"], \
      \"red_handle\":[\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\",\"0xFF0000\"],\
      \"green_handle\":[\"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\", \"0x00FF00\"], \
      \"blue_handle\":[\"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\", \"0x0000FF\"], \
      \"bl_handle\":[\"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\", \"0x000000\"] \
    }";
    // TODO
    
    CRGB KNIFE_leds[KNIFE_NUM];  //定義FastLED類別
    CRGB SHIELD_leds[SHIELD_NUM];  //定義FastLED類別
    CRGB HANDLE_leds[HANDLE_NUM];  //定義FastLED類別


    DeserializationError error;
    unsigned long starting_time_abs; // absolute time when starting
    unsigned long playing_time; // song time when playing
    unsigned long starting_time; // song time when starting
    unsigned short frame_idx; // frame index now
    bool playing; // now playing or not
  
};


#endif // LED_MANAGER
