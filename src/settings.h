// settings manager
// 29.01.2020
// Tarik2142 vk.com/tarik2142

#include "FS.h"

//#define DEBUG

const char* settingsFilename = "/8266s.cfg";

struct settings_t{
  char name[32];
  char pass[32];
  char user[32];

  float ds18b20t[8];
  float bme280t[8];
  float bme280h[8];
  float bme280p[8];
  float ccs811c[8];
  float ccs811tv[8];
  float htu21dt[8];
  float htu21dh[8];
  float mhz19c[8];

};

extern settings_t settingss{//default
  "name",
  "pass",
  "user",
  {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0},
  {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0},
  {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0},
  {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0},
  {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0},
  {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0},
  {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0},
  {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0},
  {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0},
};

bool settings_save(){
  if(!SPIFFS.begin()){
    #ifdef DEBUG
    Serial.println("SPIFFS mount failed");
    #endif
    return false;
  }
  File config = SPIFFS.open(settingsFilename, "w");
  if (!config){
    #ifdef DEBUG
    Serial.println("File open failed");
    #endif
    return false;
  }
  uint16_t bytesWrited = config.write((byte *)&settingss, sizeof(settingss));
  config.close();
  if (bytesWrited != sizeof(settingss)){
    #ifdef DEBUG
    Serial.println("writen != settings");
    #endif
    return false;
  }else{
    return true;
  }
}

bool settings_read(){
  if(!SPIFFS.begin()){
    #ifdef DEBUG
    Serial.println("SPIFFS mount failed");
    #endif
    return false;
  }
  if (!SPIFFS.exists(settingsFilename)){
    #ifdef DEBUG
    Serial.println("Config file not exsist");
    #endif
    if (!settings_save()){
      return false;
    }
  }
  File config = SPIFFS.open(settingsFilename, "r");
  if (!config){
    return false;
  }
  uint16_t bytesRead = config.read((byte *)&settingss, sizeof(settingss));
  config.close();
  if (bytesRead != sizeof(settingss)){
    return false;
  }else{
    return true;
  }
}