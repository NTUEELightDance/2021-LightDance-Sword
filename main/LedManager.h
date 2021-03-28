#ifndef LED_MANAGER
#define LED_MANAGER
#include <FastLED.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <typeinfo>

#define NAME "sword1"

#define KNIFE_NUM 60
#define SHIELD_NUM 26  
#define HANDLE_NUM 8
#define KNIFE_PIN 21  
#define SHIELD_PIN 19
#define HANDLE_PIN 18
#define BRIGHTNESS 32


class LedManager{
  public:
//    LedManager(): starting_time_abs(0), playing_time(0), playing(false) {
//    }
//    ~LedManager() {}
    


    void init() {
      FastLED.addLeds<WS2812B, KNIFE_PIN, GRB>(KNIFE_leds, KNIFE_NUM);  //設定串列全彩LED參數
      FastLED.addLeds<WS2812B, SHIELD_PIN, GRB>(SHIELD_leds, SHIELD_NUM);  //設定串列全彩LED參數
      FastLED.addLeds<WS2812B, HANDLE_PIN, GRB>(HANDLE_leds, HANDLE_NUM);  //設定串列全彩LED參數
      FastLED.setBrightness(BRIGHTNESS); // TODO: change to 255
      
      error = deserializeJson(pic_json, pic_data_test); // debug
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

    bool parsing_json(String data) {
      error = deserializeJson(led_json, data); 
//      if(error) {
//        Serial.println("Parsing Error: led_json");
//        Serial.println(error.c_str());
//        return false;
//      }
      serializeJson(led_json[1], Serial);
      serializeJson(led_json[1][0]["start"], Serial);
      Serial.println();
      Serial.println(led_json[1].size());
      return true;
    }

    bool parsing_json(const char* data) {
      error = deserializeJson(led_json, data); 
      if(error) {
        Serial.println("Parsing Error: led_json");
        Serial.println(error.c_str());
        return false;
      }
      serializeJson(led_json[1], Serial);
      Serial.println();
      Serial.println(led_json[1].size());
      return true;
    }

    void prepare_to_play(unsigned long s_time = 0, unsigned long delay_time = 0) {
      Serial.print("play from: ");
      Serial.println(s_time);
      playing_time = s_time;
      starting_time = s_time;
      set_frame_idx();
      delay(delay_time - 45);
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
      starting_time_abs = 0;
      playing_time = 0;
      frame_idx = 0;
      all_dark();
    }

    int hex_to_num(String hex, char color) {
      if(color == 'r') {
        int a = hex[2] - '0';
        int b = hex[3] - '0';
        return (10 * a + b);
      }
      else if(color == 'g') {
        int a = hex[4] - '0';
        int b = hex[5] - '0';
        return (10 * a + b);
      }
      else if(color == 'b') {
        int a = hex[6] - '0';
        int b = hex[7] - '0';
        return (10 * a + b);
      }
    }

    void show_frame() {
      Serial.println("showing frame");
      String src_sword = led_json[1][frame_idx]["status"]["LED_SWORD"]["src"]; // A name, which responds to a picture
      double alpha_sword = led_json[1][frame_idx]["status"]["LED_SWORD"]["alpha"];
      JsonObject tmp_obj = pic_json.as<JsonObject>(); // The picture at this time, every led is represented as one string, eg: "0x00FF00"
      String pic_sword = tmp_obj[src_sword];
      String src_guard = led_json[1][frame_idx]["status"]["LED_GUARD"]["src"];
      double alpha_guard = led_json[1][frame_idx]["status"]["LED_GUARD"]["alpha"];
      String pic_guard = tmp_obj[src_guard];
      String src_handle = led_json[1][frame_idx]["status"]["LED_HANDLE"]["src"];
      double alpha_handle = led_json[1][frame_idx]["status"]["LED_HANDLE"]["alpha"];
      String pic_handle = tmp_obj[src_handle];
      int now_time = led_json[1][frame_idx]["start"];

      Serial.print(src_sword);
      Serial.print(src_guard);
      Serial.println(src_handle);

      if(fade == true) {
        alpha_sword = (alpha_sword * (next_time - playing_time) + next_s * (playing_time - now_time)) / (next_time - now_time);
        alpha_guard = (alpha_guard * (next_time - playing_time) + next_g * (playing_time - now_time)) / (next_time - now_time);
        alpha_handle = (alpha_handle * (next_time - playing_time) + next_h * (playing_time - now_time)) / (next_time - now_time);
      }
      
      int led_num = 0;
      int last = 0;
      int front = 0;
      while(1) {
        if(pic_sword[front] == ' ') {
          int num = atoi(pic_sword.substring(last, front).c_str());
          for(int j = 0; j < num; j++) {
            KNIFE_leds[j+led_num].r = hex_to_num(pic_sword.substring(front + 1, front + 9), 'r') * alpha_sword;
            KNIFE_leds[j+led_num].b = hex_to_num(pic_sword.substring(front + 1, front + 9), 'b') * alpha_sword;
            KNIFE_leds[j+led_num].g = hex_to_num(pic_sword.substring(front + 1, front + 9), 'g') * alpha_sword;
          }
          if(pic_sword.length() > front + 12) {
            front = front + 10;
            last = front;
            led_num += num;
          }
          else {
            break;
          }
        }
        else {
          front += 1;
        }
      }

      led_num = 8;
      last = 0;
      front = 0;
      while(1) {
        if(pic_guard[front] == ' ') {
          int num = atoi(pic_guard.substring(last, front).c_str());
          for(int j = 0; j < num; j++) {
            SHIELD_leds[j+led_num].r = hex_to_num(pic_guard.substring(front + 1, front + 9), 'r') * alpha_guard;
            SHIELD_leds[j+led_num].b = hex_to_num(pic_guard.substring(front + 1, front + 9), 'b') * alpha_guard;
            SHIELD_leds[j+led_num].g = hex_to_num(pic_guard.substring(front + 1, front + 9), 'g') * alpha_guard;
          }
          if(pic_guard.length() > front + 12) {
            front = front + 10;
            last = front;
            led_num += num;
          }
          else {
            break;
          }
        }
        else {
          front += 1;
        }
      }

      led_num = 0;
      last = 0;
      front = 0;
      while(1) {
        if(pic_handle[front] == ' ') {
          int num = atoi(pic_handle.substring(last, front).c_str());
          for(int j = 0; j < num; j++) {
            if((j+led_num) >= 8) {
              HANDLE_leds[j+led_num - 8].r = hex_to_num(pic_handle.substring(front + 1, front + 9), 'r') * alpha_handle;
              HANDLE_leds[j+led_num - 8].b = hex_to_num(pic_handle.substring(front + 1, front + 9), 'b') * alpha_handle;
              HANDLE_leds[j+led_num - 8].g = hex_to_num(pic_handle.substring(front + 1, front + 9), 'g') * alpha_handle;
            }
            else {
              SHIELD_leds[j+led_num].r = hex_to_num(pic_handle.substring(front + 1, front + 9), 'r') * alpha_guard;
              SHIELD_leds[j+led_num].b = hex_to_num(pic_handle.substring(front + 1, front + 9), 'b') * alpha_guard;
              SHIELD_leds[j+led_num].g = hex_to_num(pic_handle.substring(front + 1, front + 9), 'g') * alpha_guard;
            }
          }
          if(pic_handle.length() > front + 12) {
            front = front + 10;
            last = front;
            led_num += num;
          }
          else {
            break;
          }
        }
        else {
          front += 1;
        }
      }

/*
      for(int i = 0; i < 59; i++) {
        KNIFE_leds[i].r = hex_to_num(pic_sword[i], 'r') * alpha_sword;
        KNIFE_leds[i].g = hex_to_num(pic_sword[i], 'g') * alpha_sword;
        KNIFE_leds[i].b = hex_to_num(pic_sword[i], 'b') * alpha_sword;
      }
      for(int i = 0; i < 8; i++) {
        SHIELD_leds[i].r = hex_to_num(pic_handle[i], 'r') * alpha_guard;
        SHIELD_leds[i].g = hex_to_num(pic_handle[i], 'g') * alpha_guard;
        SHIELD_leds[i].b = hex_to_num(pic_handle[i], 'b') * alpha_guard;
      }
      for(int i = 0; i < 18; i++) {
        SHIELD_leds[i+8].r = hex_to_num(pic_guard[i], 'r') * alpha_guard;
        SHIELD_leds[i+8].g = hex_to_num(pic_guard[i], 'g') * alpha_guard;
        SHIELD_leds[i+8].b = hex_to_num(pic_guard[i], 'b') * alpha_guard;
      }
      for(int i = 0; i < 8; i++) {
        HANDLE_leds[i].r = hex_to_num(pic_handle[i+8], 'r') * alpha_handle;
        HANDLE_leds[i].g = hex_to_num(pic_handle[i+8], 'g') * alpha_handle;
        HANDLE_leds[i].b = hex_to_num(pic_handle[i+8], 'b') * alpha_handle;
      }
*/


      FastLED.show();
      Serial.println(playing_time);
    }

    void light_current_status(DynamicJsonDocument data) {
//      error = deserializeJson(tmp_json, data);
//      if(error) {
//        Serial.println("Parsing Error: status");
//        Serial.println(error.c_str());
//      }
//      status = tmp_json[1];
//      status = data;
      JsonObject tmp_data = data.as<JsonObject>();
      String src_sword = tmp_data["LED_SWORD"]["src"]; 
      double alpha_sword = tmp_data["LED_SWORD"]["alpha"];
      String src_guard = tmp_data["LED_GUARD"]["src"]; 
      double alpha_guard = tmp_data["LED_GUARD"]["alpha"];
      String src_handle = data["LED_HANDLE"]["src"]; 
      double alpha_handle = data["LED_HANDLE"]["alpha"];

      JsonObject tmp_obj = pic_json.as<JsonObject>(); // The picture at this time, every led is represented as one string, eg: "0x00FF00"
      String pic_sword = tmp_obj[src_sword];
      String pic_guard = tmp_obj[src_guard];
      String pic_handle = tmp_obj[src_handle];
      
      int led_num = 0;
      int last = 0;
      int front = 0;
      while(1) {
        if(pic_sword[front] == ' ') {
          int num = atoi(pic_sword.substring(last, front).c_str());
          for(int j = 0; j < num; j++) {
            KNIFE_leds[j+led_num].r = hex_to_num(pic_sword.substring(front + 1, front + 9), 'r') * alpha_sword;
            KNIFE_leds[j+led_num].b = hex_to_num(pic_sword.substring(front + 1, front + 9), 'b') * alpha_sword;
            KNIFE_leds[j+led_num].g = hex_to_num(pic_sword.substring(front + 1, front + 9), 'g') * alpha_sword;
          }
          if(pic_sword.length() > front + 12) {
            front = front + 10;
            last = front;
            led_num += num;
          }
          else {
            break;
          }
        }
        else {
          front += 1;
        }
      }

      led_num = 8;
      last = 0;
      front = 0;
      while(1) {
        if(pic_guard[front] == ' ') {
          int num = atoi(pic_guard.substring(last, front).c_str());
          for(int j = 0; j < num; j++) {
            SHIELD_leds[j+led_num].r = hex_to_num(pic_guard.substring(front + 1, front + 9), 'r') * alpha_guard;
            SHIELD_leds[j+led_num].b = hex_to_num(pic_guard.substring(front + 1, front + 9), 'b') * alpha_guard;
            SHIELD_leds[j+led_num].g = hex_to_num(pic_guard.substring(front + 1, front + 9), 'g') * alpha_guard;
          }
          if(pic_guard.length() > front + 12) {
            front = front + 10;
            last = front;
            led_num += num;
          }
          else {
            break;
          }
        }
        else {
          front += 1;
        }
      }

      led_num = 0;
      last = 0;
      front = 0;
      while(1) {
        if(pic_handle[front] == ' ') {
          int num = atoi(pic_handle.substring(last, front).c_str());
          for(int j = 0; j < num; j++) {
            if((j+led_num) >= 8) {
              HANDLE_leds[j+led_num - 8].r = hex_to_num(pic_handle.substring(front + 1, front + 9), 'r') * alpha_handle;
              HANDLE_leds[j+led_num - 8].b = hex_to_num(pic_handle.substring(front + 1, front + 9), 'b') * alpha_handle;
              HANDLE_leds[j+led_num - 8].g = hex_to_num(pic_handle.substring(front + 1, front + 9), 'g') * alpha_handle;
            }
            else {
              SHIELD_leds[j+led_num].r = hex_to_num(pic_handle.substring(front + 1, front + 9), 'r') * alpha_guard;
              SHIELD_leds[j+led_num].b = hex_to_num(pic_handle.substring(front + 1, front + 9), 'b') * alpha_guard;
              SHIELD_leds[j+led_num].g = hex_to_num(pic_handle.substring(front + 1, front + 9), 'g') * alpha_guard;
            }
          }
          if(pic_handle.length() > front + 12) {
            front = front + 10;
            last = front;
            led_num += num;
          }
          else {
            break;
          }
        }
        else {
          front += 1;
        }
      }
      
      FastLED.show();
    }

    void set_frame_idx() {
      // To set the frame index, according to the playing time and the timeline
      while(!frame_end() && playing_time >= led_json[1][frame_idx + 1]["start"]) {
        frame_idx += 1;
      }
      while(frame_idx != 0 && playing_time < led_json[1][frame_idx]["start"]) {
        frame_idx -= 1;
      }
    }

    bool frame_end() {
      if(frame_idx == led_json[1].size() - 1){
        Serial.println("end of frame");
        return true;
      }
      return false;
    }

    void all_dark() {
      for(int i = 0; i < KNIFE_NUM; i++) {
        KNIFE_leds[i] = strtol("0x000000", NULL, 0);
      }
      for(int i = 0; i < SHIELD_NUM; i++) {
        SHIELD_leds[i] = strtol("0x000000", NULL, 0);
      }
      for(int i = 0; i < HANDLE_NUM; i++) {
        HANDLE_leds[i] = strtol("0x000000", NULL, 0);
      }
      FastLED.show();
    }

    void loop() {
      if(playing) {
        // Serial.println("now playing");
        playing_time = millis() - starting_time_abs + starting_time;
        // Serial.print("playing_time: ");
        // Serial.print(playing_time);
        // Serial.print("starting_time_abs: ");
        // Serial.println(starting_time_abs);
        // Serial.print("frame_idx:");
        // Serial.print(frame_idx);
        // Serial.print(" led_json[1][frame_idx][\"start\"]:");
        // serializeJson(led_json[1][frame_idx]["start"], Serial);
        // Serial.println();
        if(frame_end()) {
          playing = false;
          starting_time_abs = 0;
          playing_time = 0;
          frame_idx = 0;
          return;
        }

        else if (fade == true) {
          show_frame();
        }

        if(playing_time > led_json[1][frame_idx+1]["start"]){
          Serial.print(playing_time);
          Serial.print("  ");
          serializeJson(led_json[1][frame_idx+1]["start"], Serial);
          Serial.println();
          frame_idx = frame_idx + 1;
          Serial.println("showing frame");
          if(led_json[1][frame_idx]["fade"] == true) {
            fade = true;
            next_s = led_json[1][frame_idx+1]["status"]["LED_SWORD"]["alpha"];
            next_g = led_json[1][frame_idx+1]["status"]["LED_GUARD"]["alpha"];
            next_h = led_json[1][frame_idx+1]["status"]["LED_HANDLE"]["alpha"];
            next_time = led_json[1][frame_idx+1]["start"];
          }
          else {
            fade = false;
          }
          show_frame();
        }
        else if (led_json[1][0]["start"] == 0 && playing_time == 0) {
          show_frame();
        }
      }
    }
    


  private:
    StaticJsonDocument<70000> led_json;
    StaticJsonDocument<10000> pic_json; 
    StaticJsonDocument<200> status;

//    JsonArray status;
//    StaticJsonDocument<5000> tmp_json;

    const char* pic_data_test = "{\
      \"bl_sword\": \"59 0x000000\", \
      \"bl_guard\": \"18 0x000000\", \
      \"bl_handle\": \"16 0x000000\", \
      \"white_sword\": \"59 0xFFFFFF\", \
      \"white_guard\": \"18 0xFFFFFF\", \
      \"white_handle\": \"16 0xFFFFFF\", \
      \"red_sword\": \"59 0xFF0000\", \
      \"red_guard\": \"18 0xFF0000\", \
      \"red_handle\": \"16 0xFF0000\", \
      \"blue_sword\": \"59 0x0000FF\", \
      \"blue_guard\": \"18 0x0000FF\", \
      \"blue_handle\": \"16 0x0000FF\", \
      \"green_sword\": \"59 0x00FF00\", \
      \"green_guard\": \"18 0x00FF00\", \
      \"green_handle\": \"16 0x00FF00\", \
      \"purple_sword\": \"59 0xFF00FF\", \
      \"purple_guard\": \"18 0xFF00FF\", \
      \"purple_handle\": \"16 0xFF00FF\", \
      \"yellow_sword\": \"59 0xFFFF00\", \
      \"yellow_guard\": \"18 0xFFFF00\", \
      \"yellow_handle\": \"16 0xFFFF00\", \
      \"red_sword_1\":\"3 0xFF0000 54 0x000000 2 0xFF0000 \", \
      \"red_sword_2\":\"1 0x000000 3 0xFF0000 52 0x000000 3 0xFF0000\", \
      \"red_sword_3\":\"2 0x000000 3 0xFF0000 50 0x000000 3 0xFF0000 1 0x000000\", \
      \"red_sword_4\":\"3 0x000000 3 0xFF0000 48 0x000000 3 0xFF0000 2 0x000000\", \
      \"red_sword_5\":\"4 0x000000 3 0xFF0000 46 0x000000 3 0xFF0000 3 0x000000\", \
      \"red_sword_6\":\"5 0x000000 3 0xFF0000 44 0x000000 3 0xFF0000 4 0x000000\", \
      \"red_sword_7\":\"6 0x000000 3 0xFF0000 42 0x000000 3 0xFF0000 5 0x000000\", \
      \"red_sword_8\":\"7 0x000000 3 0xFF0000 40 0x000000 3 0xFF0000 6 0x000000\", \
      \"red_sword_9\":\"8 0x000000 3 0xFF0000 38 0x000000 3 0xFF0000 7 0x000000\", \
      \"red_sword_10\":\"9 0x000000 3 0xFF0000 36 0x000000 3 0xFF0000 8 0x000000\", \
      \"red_sword_11\":\"10 0x000000 3 0xFF0000 34 0x000000 3 0xFF0000 9 0x000000\", \
      \"red_sword_12\":\"11 0x000000 3 0xFF0000 32 0x000000 3 0xFF0000 10 0x000000\", \
      \"red_sword_13\":\"12 0x000000 3 0xFF0000 30 0x000000 3 0xFF0000 11 0x000000\", \
      \"red_sword_14\":\"13 0x000000 3 0xFF0000 28 0x000000 3 0xFF0000 12 0x000000\", \
      \"red_sword_15\":\"14 0x000000 3 0xFF0000 26 0x000000 3 0xFF0000 13 0x000000\", \
      \"red_sword_16\":\"15 0x000000 3 0xFF0000 24 0x000000 3 0xFF0000 14 0x000000\", \
      \"red_sword_17\":\"16 0x000000 3 0xFF0000 22 0x000000 3 0xFF0000 15 0x000000\", \
      \"red_sword_18\":\"17 0x000000 3 0xFF0000 20 0x000000 3 0xFF0000 16 0x000000\", \
      \"red_sword_19\":\"18 0x000000 3 0xFF0000 18 0x000000 3 0xFF0000 17 0x000000\", \
      \"red_sword_20\":\"19 0x000000 3 0xFF0000 16 0x000000 3 0xFF0000 18 0x000000\", \
      \"red_sword_21\":\"20 0x000000 3 0xFF0000 14 0x000000 3 0xFF0000 19 0x000000\", \
      \"red_sword_22\":\"21 0x000000 3 0xFF0000 12 0x000000 3 0xFF0000 20 0x000000\", \
      \"red_sword_23\":\"22 0x000000 3 0xFF0000 10 0x000000 3 0xFF0000 21 0x000000\", \
      \"red_sword_24\":\"23 0x000000 3 0xFF0000 8 0x000000 3 0xFF0000 22 0x000000\", \
      \"red_sword_25\":\"24 0x000000 3 0xFF0000 6 0x000000 3 0xFF0000 23 0x000000\", \
      \"red_sword_26\":\"25 0x000000 3 0xFF0000 4 0x000000 3 0xFF0000 24 0x000000\", \
      \"red_sword_27\":\"26 0x000000 3 0xFF0000 2 0x000000 3 0xFF0000 25 0x000000\", \
      \"red_sword_28\":\"27 0x000000 6 0xFF0000 26 0x000000\", \
      \"red_sword_ending_1\":\"26 0xFF0000 7 0x000000 26 0xFF0000\", \
      \"red_sword_ending_2\":\"22 0xFF0000 15 0x000000 22 0xFF0000\", \
      \"red_sword_ending_3\":\"18 0xFF0000 23 0x000000 18 0xFF0000\", \
      \"red_sword_ending_4\":\"14 0xFF0000 31 0x000000 14 0xFF0000\", \
      \"red_sword_ending_5\":\"10 0xFF0000 39 0x000000 10 0xFF0000\", \
      \"red_sword_ending_6\":\"6 0xFF0000 47 0x000000 6 0xFF0000\" \
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
    bool fade; // fade from one frame to the next
    double next_s; // sword next frame alpha
    double next_g; // guard next frame alpha
    double next_h; // handle next frame alpha
    int next_time; // time from this frame to the next frame
};


#endif // LED_MANAGER
