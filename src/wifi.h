#include <Arduino.h>
#include <FS.h>  

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
WiFiManager wifiManager;
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

char mqtt_server[40] = "mqtt-155.ztx.com.ua";
char mqtt_port[6] = "1883";
char blynk_token[100] = "YOUR_EMAIL";

bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  #ifdef DEBUGA
  Serial.println("Should save config");
  #endif
  shouldSaveConfig = true;
}
void wiffi(){
  if (SPIFFS.begin()) {
    #ifdef DEBUGA
    Serial.println("mounted file system");
  #endif
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      #ifdef DEBUGA
      Serial.println("reading config file");
      #endif
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        #ifdef DEBUGA
        Serial.println("opened config file");
        
  #endif
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        #ifdef DEBUGA

        json.printTo(Serial);

  #endif
        if (json.success()) {
          #ifdef DEBUGA

          Serial.println("\nparsed json");
  #endif
          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(blynk_token, json["blynk_token"]);

        } else {
          #ifdef DEBUGA
          Serial.println("failed to load json config");
      
  #endif
        }
        configFile.close();
      }
    }
  } else {
    #ifdef DEBUGA

    Serial.println("failed to mount FS");

  #endif
  }
  //end read



  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
  WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 100);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set static ip
  //wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  
  //add all your parameters here
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_blynk_token);

wifiManager.setConfigPortalTimeout (60);
  snprintf(mac, 120, "ztx_eco_%02x%02x%02x", macb[3], macb[4], macb[5]);
  if (!wifiManager.autoConnect(mac)) {
    #ifdef DEBUGA
    Serial.println("failed to connect and hit timeout");
  #endif
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  #ifdef DEBUGA
  Serial.println("connected...yeey :)");
  #endif
  //read updated parameters
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(blynk_token, custom_blynk_token.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    #ifdef DEBUGA
    Serial.println("saving config");

  #endif
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["blynk_token"] = blynk_token;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      #ifdef DEBUGA
      Serial.println("failed to open config file for writing");
  #endif
    }
#ifdef DEBUGA
    json.printTo(Serial);
  #endif
    json.printTo(configFile);
    configFile.close();
    //end save
  }
#ifdef DEBUGA
  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  Serial.println(mqtt_server);
  Serial.println(mqtt_port);
  Serial.println(blynk_token);

  #endif
}
void wiffireset(){
wifiManager.resetSettings();
ESP.reset();
}
