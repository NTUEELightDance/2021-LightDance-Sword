#ifndef LED_MANAGER
#define LED_MANAGER
#include <FastLED.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <typeinfo>

// led numbers, led pins, brightness info
#define KNIFE_NUM 60
#define GUARD_NUM 26  
#define HANDLE_NUM 8
#define KNIFE_PIN 27  
#define GUARD_PIN 26
#define HANDLE_PIN 25
#define BRIGHTNESS 32

class LedManager{
  public:
    void init() {
      FastLED.addLeds<WS2812B, KNIFE_PIN, GRB>(KNIFE_leds, KNIFE_NUM);  //設定串列全彩LED參數
      FastLED.addLeds<WS2812B, GUARD_PIN, GRB>(GUARD_leds, GUARD_NUM);  //設定串列全彩LED參數
      FastLED.addLeds<WS2812B, HANDLE_PIN, GRB>(HANDLE_leds, HANDLE_NUM);  //設定串列全彩LED參數
      FastLED.setBrightness(BRIGHTNESS); 
      
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
      /*
      Get the timeline and the patterns.
      */
      error = deserializeJson(led_json, data); 
      serializeJson(led_json[1], Serial);
      serializeJson(led_json[1][0]["start"], Serial);
      Serial.println();
      Serial.println(led_json[1].size());
      return true;
    }


    bool parsing_json(const char* data) {
      /*
      Get the timeline and the patterns.
      */
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
      if(delay_time > 45) {
        delay(delay_time - 45);
      }
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

      if(src_sword == NULL) {
        return;
      }
      Serial.print(src_sword);
      Serial.print(src_guard);
      Serial.println(src_handle);

      // Dealing with the fade effect. 
      // Using linear method to decide the brightness.
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
            GUARD_leds[j+led_num].r = hex_to_num(pic_guard.substring(front + 1, front + 9), 'r') * alpha_guard;
            GUARD_leds[j+led_num].b = hex_to_num(pic_guard.substring(front + 1, front + 9), 'b') * alpha_guard;
            GUARD_leds[j+led_num].g = hex_to_num(pic_guard.substring(front + 1, front + 9), 'g') * alpha_guard;
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
              GUARD_leds[j+led_num].r = hex_to_num(pic_handle.substring(front + 1, front + 9), 'r') * alpha_guard;
              GUARD_leds[j+led_num].b = hex_to_num(pic_handle.substring(front + 1, front + 9), 'b') * alpha_guard;
              GUARD_leds[j+led_num].g = hex_to_num(pic_handle.substring(front + 1, front + 9), 'g') * alpha_guard;
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
      Serial.println(playing_time);
    }


    void light_current_status(DynamicJsonDocument data) {
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
            GUARD_leds[j+led_num].r = hex_to_num(pic_guard.substring(front + 1, front + 9), 'r') * alpha_guard;
            GUARD_leds[j+led_num].b = hex_to_num(pic_guard.substring(front + 1, front + 9), 'b') * alpha_guard;
            GUARD_leds[j+led_num].g = hex_to_num(pic_guard.substring(front + 1, front + 9), 'g') * alpha_guard;
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
              GUARD_leds[j+led_num].r = hex_to_num(pic_handle.substring(front + 1, front + 9), 'r') * alpha_guard;
              GUARD_leds[j+led_num].b = hex_to_num(pic_handle.substring(front + 1, front + 9), 'b') * alpha_guard;
              GUARD_leds[j+led_num].g = hex_to_num(pic_handle.substring(front + 1, front + 9), 'g') * alpha_guard;
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
      Serial.println(frame_idx);
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
        GUARD_leds[i] = strtol("0x000000", NULL, 0);
      }
      for(int i = 0; i < HANDLE_NUM; i++) {
        HANDLE_leds[i] = strtol("0x000000", NULL, 0);
      }
      FastLED.show();
    }


    void loop() {
      if(playing) {
        playing_time = millis() - starting_time_abs + starting_time;

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
          try {
            double test = led_json[1][frame_idx+1]["status"]["LED_SWORD"]["alpha"];
          }
          catch (const char e) {
            Serial.println("data error");
            frame_idx = frame_idx + 1;
            return;
          }
          Serial.print(playing_time);
          Serial.print("  ");
          serializeJson(led_json[1][frame_idx+1]["start"], Serial);
          Serial.println();
          frame_idx = frame_idx + 1;

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
      \"red_sword_shadow_1\":\"4 0xFF0000 51 0x000000 4 0xFF0000\", \
      \"red_sword_shadow_2\":\"4 0x000000 4 0xFF0000 43 0x000000 4 0xFF0000 4 0x000000\", \
      \"red_sword_shadow_3\":\"8 0x000000 4 0xFF0000 35 0x000000 4 0xFF0000 8 0x000000\", \
      \"red_sword_shadow_4\":\"12 0x000000 4 0xFF0000 27 0x000000 4 0xFF0000 12 0x000000\", \
      \"red_sword_shadow_5\":\"16 0x000000 4 0xFF0000 19 0x000000 4 0xFF0000 16 0x000000\", \
      \"red_sword_shadow_6\":\"20 0x000000 5 0xFF0000 9 0x000000 5 0xFF0000 20 0x000000\", \
      \"red_sword_shadow_7\":\"24 0x000000 11 0xFF0000 24 0x000000\", \
      \"white_sword_1\":\"7 0xFFFFFF 45 0x000000 7 0xFFFFFF\", \
      \"white_sword_2\":\"14 0xFFFFFF 31 0x000000 14 0xFFFFFF\", \
      \"white_sword_3\":\"21 0xFFFFFF 17 0x000000 21 0xFFFFFF\", \
      \"white_sword_ending1\":\"26 0xFFFFFF 7 0x000000 26 0xFFFFFF\", \
      \"white_sword_ending2\":\"22 0xFFFFFF 15 0x000000 22 0xFFFFFF\", \
      \"white_sword_ending3\":\"18 0xFFFFFF 23 0x000000 18 0xFFFFFF\", \
      \"white_sword_ending4\":\"14 0xFFFFFF 31 0x000000 14 0xFFFFFF\", \
      \"white_sword_ending5\":\"10 0xFFFFFF 39 0x000000 10 0xFFFFFF\", \
      \"white_sword_ending6\":\"6 0xFFFFFF 47 0x000000 6 0xFFFFFF\", \
      \"white_sword_ending7\":\"2 0xFFFFFF 55 0x000000 2 0xFFFFFF\" \
    }";

    CRGB KNIFE_leds[KNIFE_NUM];  //定義FastLED類別
    CRGB GUARD_leds[GUARD_NUM];  //定義FastLED類別
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
