#include "WiFiClientSecure.h"
#include "ESP8266httpUpdate.h"
//#define DEBUGA

//const char* downloadHost = "firebasestorage.googleapis.com";
const int16_t httpsPort = 443;
//const char* downloadEnding = "?alt=media";
enum updateResult_t : uint8_t {OTA_OK, OTA_CONNFAIL, OTA_UPDFAIL, OTA_NOFIRMWARE, OTA_NONEWFIRMWARE};//0, 1, 2, 3, 4
///v0/b/tarik2142cloud.appspot.com/o/esp8266%2Fota%2Ffirmware.bin?alt=media

updateResult_t checkUpdates(const char* downloadHost, const char* firmwarelink){//OTA_OK, OTA_CONNFAIL, OTA_UPDFAIL, OTA_NOFIRMWARE, OTA_NONEWFIRMWARE
    BearSSL::WiFiClientSecure clientS;
    clientS.setInsecure();
    clientS.setTimeout(1000);
    #ifdef DEBUGA
    Serial.print("connecting to ");
    Serial.println(downloadHost);
    #endif
    if (!clientS.connect(downloadHost, httpsPort)) {
        #ifdef DEBUGA
        Serial.println("connection failed");
        #endif
        return OTA_CONNFAIL;
    }else{
        #ifdef DEBUGA
        Serial.println("connected!");
        #endif
        clientS.print(String("GET ") + firmwarelink + " HTTP/1.1\r\n" +
            "Host: " + downloadHost + "\r\n" +
            "User-Agent: esp8266\r\n" +
            "Connection: close\r\n\r\n");
        #ifdef DEBUGA
        Serial.println("request sent");
        #endif
        while (clientS.connected()) {
            String line = clientS.readStringUntil('\n');
            if (line.indexOf("404") > 0) {
                #ifdef DEBUGA
                Serial.println("firmware not found");
                Serial.println(line);
                #endif
            return OTA_NOFIRMWARE;
            } 
            if (line == "\r") {
                #ifdef DEBUGA
                Serial.println("headers received");
                #endif
                break;
            }
        }     
        #ifdef DEBUGA
        Serial.println("firmware found");
        #endif
        HTTPClient http;
        clientS.stop();
        //const int size = strlen(firmwarelink) + strlen(downloadEnding) + 1;
        //char link[size];
        //snprintf(link, size, "%s%s", firmwarelink, downloadEnding);
        if (http.begin(clientS, downloadHost, httpsPort, firmwarelink)){
            #ifdef DEBUGA
            Serial.println("http client started");
            #endif
            int16_t httpCode = http.GET();
            if (httpCode == HTTP_CODE_OK){
                const int len = http.getSize();
                if (len > 0){
                    if (len != ESP.getSketchSize()){
                        #ifdef DEBUGA
                        Serial.println("found new firmware");
                        Serial.print("new firmware len = ");
                        Serial.print(len);
                        Serial.println();
                        Serial.print("old firmware len = ");
                        Serial.print(ESP.getSketchSize());
                        Serial.println();
                        #endif
                        http.end();
                        return OTA_OK;
                    }else{
                        #ifdef DEBUGA
                        Serial.println("no new firmware found");
                        #endif
                        http.end();
                        return OTA_NONEWFIRMWARE;
                    }
                }else{
                    #ifdef DEBUGA
                    Serial.println("len < 0");
                    #endif
                    http.end();
                    return OTA_UPDFAIL;
                }    
            }
            #ifdef DEBUGA
                Serial.println();
                Serial.print("HTTP_CODE: ");
                Serial.print(httpCode);
                Serial.println();
            #endif
            http.end();
            return OTA_CONNFAIL;
        }else{
            #ifdef DEBUGA
            Serial.println("http client fail");
            #endif
            http.end();
            return OTA_CONNFAIL;
        }
    }
    return OTA_CONNFAIL;
}

updateResult_t startUpdate(const char* downloadHost, const char* firmwarelink){//OTA_OK, OTA_CONNFAIL, OTA_UPDFAIL, OTA_NOFIRMWARE, OTA_NONEWFIRMWARE
    int size = strlen(downloadHost) + strlen(firmwarelink) + 10;
    char link[size];
    snprintf(link, size, "https://%s%s", downloadHost, firmwarelink);
    #ifdef DEBUGA
    Serial.print("Starting OTA from: ");
    Serial.println(link);
    #endif
    BearSSL::WiFiClientSecure clientS;
    clientS.setInsecure();
    clientS.setTimeout(1000);
    #ifdef DEBUGA
    Serial.print("connecting to ");
    Serial.println(downloadHost);
    #endif
    if (!clientS.connect(downloadHost, httpsPort)) {
        #ifdef DEBUGA
        Serial.println("connection failed");
        #endif
        return OTA_CONNFAIL;
    }else{
        #ifdef DEBUGA
        Serial.println("connected!");
        #endif
    }
    auto ret = ESPhttpUpdate.update(clientS, link);
    #ifdef DEBUGA
    switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
  }
    #endif
    return OTA_UPDFAIL;
}
