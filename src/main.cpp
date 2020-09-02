#define DS18B20ON //data d3
#define BME280ON //sda D2, scl D1
#define CCS811ON //sda D2, scl D1
#define HTU21DON //sda D2, scl D1
#define MHZ19ON // D7=13-rx, D8=15-tx 
// SW Serial
#define SERIAL_RX 13
#define SERIAL_TX 15

#define DEBUGA
#define DEEPSLEEEP
#include "settings.h"
#include <Wire.h>

const char *ver = "1.1test4";
//const char *config = "BME280_nosleep";const char *firmware = "/ztxdevelopers/ota_esp8266/-/raw/master/sensors/BME280/firmware.bin";
//const char *config = "BME280_CCS811_MHZ19_nosleep_nocalib";const char *firmware = "/ztxdevelopers/ota_esp8266/-/raw/master/sensors/BME280ON_CCS811ON_MHZ19ON/firmware.bin";
//const char *config = "BME280_MHZ19_nosleep_nocalib";const char *firmware = "/ztxdevelopers/ota_esp8266/-/raw/master/sensors/BME280_MHZ19/firmware.bin";
//const char *config = "BME280_DS18B20_nosleep";const char *firmware = "/ztxdevelopers/ota_esp8266/-/raw/master/sensors/BME280_DS18B20/firmware.bin";
//const char *config = "DS18B20_nosleep";const char *firmware = "/ztxdevelopers/ota_esp8266/-/raw/master/sensors/DS18B20/firmware.bin";
//const char *config = "HTU21D_nosleep";const char *firmware = "/ztxdevelopers/ota_esp8266/-/raw/master/sensors/HTU21D/firmware.bin";
//const char *config = "HTU21D_MHZ19_nosleep_nocalib";const char *firmware = "/ztxdevelopers/ota_esp8266/-/raw/master/sensors/HTU21D_MHZ19/firmware.bin";
const char *config = "oll_sleep60s_tocalib";
const char *firmware = "/ztxdevelopers/ota_esp8266/-/raw/master/sensors/ALL/firmware.bin";
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SmartDelay.h>
SmartDelay foo(1000000UL);
#include <ezTime.h>
Timezone myTZ;
const int LED(D4);

#define MSG_BUFFER_SIZE (10)
char msg[MSG_BUFFER_SIZE];
#define TOPIK_BUFFER_SIZE (100)
char topik[TOPIK_BUFFER_SIZE];
char mdebug[TOPIK_BUFFER_SIZE];
char mac[120];
int stan = 0;
char namer[32]="resetSettings";
char calibr[32]="calibr";
char nocalibr[32]="nocalibr";
byte macb[6];
WiFiClient espClient;
PubSubClient client(espClient);

#include "firebaseUpdate.h"
const char *host = "gitlab.com";
//#include <ESP8266httpUpdate.h>
void update(const char *downloadHost, const char *firmwarelink)
{
  snprintf(topik, TOPIK_BUFFER_SIZE, "%s/sys/cmd/update", mac);
  if (checkUpdates(downloadHost, firmwarelink) == OTA_OK)
  {
    client.publish(topik, "0", true);
    startUpdate(downloadHost, firmwarelink);
 }
}

#ifdef BME280ON
//#include <Wire.h>
#include "BlueDot_BME280.h"
BlueDot_BME280 bme;
#endif
#ifdef HTU21DON
//#include <Wire.h>
//#include "SparkFunHTU21D.h"
//HTU21D myHumidity;
#include <HTU21D.h>
HTU21D myHTU21D(HTU21D_RES_RH12_TEMP14);

#endif
#ifdef CCS811ON
//#include <Wire.h>
#define CCS811_ADDR 0x5B 
#include "SparkFunCCS811.h"
CCS811 myCCS811(CCS811_ADDR);
#endif

#ifdef DS18B20ON
#include <OneWire.h>
#include <DallasTemperature.h>
OneWire oneWire(D3); // Пин к которому подключен датчик
DallasTemperature ds(&oneWire);
byte numb_of_sens;

DeviceAddress insideThermometer; // тут будет хранится адрес нашего датчика. Каждый DS18B20 имеет свой уникальный адрес, зашитый на заводе
#endif
#ifdef MHZ19ON
#include <MHZ19.h>
MHZ19 mhz19(SERIAL_RX, SERIAL_TX);
#endif
#include "wifi.h"
/*const char* paytochar(uint8_t* paylo,int length){
  String str="";
 char c[length];
  for(int i=0;i<length;i++){
    //c=(char)(*(paylo+i));
    str +=(char)(*(paylo+i));
  }
for(int i=0;i<length;i++){

    c[i] +=(char)(*(paylo+i));
  }
  Serial.println(str);
  if (str.startsWith("ink0gitlab.com/")){
    str.replace("ink0gitlab.com/","/");
    char chah[str.length()];
    str.toCharArray(chah,str.length()); 
    return c;
  }else{
    return "as";
  }
}*/
float paytofloat(uint8_t *paylo, int length)
{
  String str = "";
  char c;
  for (int i = 0; i < length; i++)
  {
    if (isDigit(c = (char)(*(paylo + i))))
    {
      str += c;
    }
    else if (isPunct(c = (char)(*(paylo + i))))
    {
      str += c;
    }
  }
  return str.toFloat();
}
const char *dat[]={"DS18B20","BME280","CCS811","HTU21D","MHZ19"};
const char *param[]={"Temperature_C","Humidity","Pressure","CO2","TVOC"};
void callback(char *topic, uint8_t *payload, unsigned int length)
{
  float val = paytofloat(payload, length);
  #ifdef DEBUGA
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(val);
  #endif
  String stopic = String(topic);
  if (stopic.endsWith("update"))
  {
    #ifdef DEBUGA
    Serial.println("update");
    #endif
    if (payload[0] == '1')
    {
      update(host, firmware);
    }
  
  }else if (stopic.endsWith("resetSettings"))
  {
    #ifdef DEBUGA
    Serial.println("resetSettings");
    #endif
    if (payload[0] == '1')
    {
      //client.publish(topic, "0", true);
      //resSettings=true;
      
      if (strcmp(settingss.name, namer) != 0){
      strncpy(settingss.name, namer, sizeof(settingss.name));
      settings_save();}
      snprintf(topik, TOPIK_BUFFER_SIZE, "%s/sys/cmd/resetSettings", mac);
      client.publish(topik, "0", true);
    }
  }else if (stopic.endsWith("MHZ19/calibr"))
  {
    #ifdef DEBUGA
    Serial.println("calib");
    #endif
    if (payload[0] == '1')
    {
      if (strcmp(settingss.pass, calibr) != 0){
      strncpy(settingss.pass, calibr, sizeof(settingss.pass));
      settings_save();}
    }else 
    {
      if (strcmp(settingss.pass, nocalibr) != 0){
      strncpy(settingss.pass, nocalibr, sizeof(settingss.pass));
      settings_save();}
    }
    delay(1000);
    ESP.restart();
  }else if (stopic.endsWith("reboot"))
  {
    #ifdef DEBUGA
    Serial.println("reboot");
    #endif
  
    if (payload[0] == '1')
    {
      snprintf(topik, TOPIK_BUFFER_SIZE, "%s/sys/cmd/reboot", mac);
      client.publish(topik, "0", true);
      ESP.restart();
      client.publish(topik, "2", true);
    }
    
  }else{
    for (int i=0;i<5;i++){//dat[]
      for (int ii=0;ii<8;ii++){//nomer
        for (int iii=0;iii<5;iii++){//param[]
          snprintf(topik, TOPIK_BUFFER_SIZE, "%s/%d/%s", dat[i], ii, param[iii]);
          if (stopic.endsWith(topik)){
            #ifdef DEBUGA
            Serial.println(topik);
            Serial.println(i);
            Serial.println(ii);
            Serial.println(iii);
            #endif
            switch (i)
            {
            case 0://DS18B20  settingss.name = "";
              if (iii==0&&settingss.ds18b20t[ii] != val){settingss.ds18b20t[ii] = val;}//Temperature_C
              break;
            case 1://BME280
              if (iii==0&&settingss.bme280t[ii] != val){settingss.bme280t[ii] = val;}//Temperature_C
              else if(iii==1&&settingss.bme280h[ii] != val){settingss.bme280h[ii] = val;}//Humidity
              else if(iii==2&&settingss.bme280p[ii] != val){settingss.bme280p[ii] = val;}//Pressure
              break;
            case 2://CCS811
              if (iii==3&&settingss.ccs811c[ii] != val){settingss.ccs811c[ii] = val;}//CO2
              else if(iii==4&&settingss.ccs811tv[ii] != val){settingss.ccs811tv[ii] = val;}//TVOC
              break;
            case 3://HTU21D
              if (iii==0&&settingss.htu21dt[ii] != val){settingss.htu21dt[ii] = val;}//Temperature_C
              else if(iii==1&&settingss.htu21dh[ii] != val){settingss.htu21dh[ii] = val;}//Humidity
              break;
            case 4://MHZ19
              if (iii==3&&settingss.mhz19c[ii] != val){settingss.mhz19c[ii] = val;}//CO2
              break;
            default:
              break;
            }
            settings_save();
          }
        }
      }
    }
  }
}
void reconnect()
{
  while (!client.connected())
  {
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), "nano", "nano019283"))
    {
      #ifdef DEBUGA
      Serial.println("mqtt conected");
      #endif
      snprintf(topik, TOPIK_BUFFER_SIZE, "%s/sys/ver", mac);
      client.publish(topik, ver,true);
      snprintf(topik, TOPIK_BUFFER_SIZE, "%s/sys/cmd/#", mac);
      client.subscribe(topik);
      snprintf(topik, TOPIK_BUFFER_SIZE, "%s/sys/config", mac);
      client.publish(topik, config,true);
      snprintf(topik, TOPIK_BUFFER_SIZE, "%s/sys/MHZ19/calibr", mac);
      if (strcmp(settingss.pass, nocalibr) == 0){
        client.publish(topik, "OFF",true);
      }else{
        client.publish(topik, "ON",true);
      }
    }
    else
    {
      #ifdef DEBUGA
      Serial.println("mqtt disconected, reconnecting ... ");
      #endif
      digitalWrite(LED, LOW);
      delay(800);
      digitalWrite(LED, HIGH);
      delay(200);
      
    }
  }
}
void BME280setup()
{
#ifdef BME280ON

  bme.parameter.communication = 0; //I2C communication for Sensor
  bme.parameter.I2CAddress = 0x76; //0x77 I2C Address for Sensor

  // Выбор режима работаты датчика
  // 0b00: в режиме ожидания измерения не выполняются, но энергопотребление минимально (sleep mode)
  // 0b01: в принудительном режиме выполняется одиночное измерение, и устройство автоматически возвращается в спящий режим (forced mode)
  // 0b11: В нормальном режиме датчик постоянно измеряет (значение по умолчанию) (normal mode)

  bme.parameter.sensorMode = 0b11; //Setup Sensor mode for Sensor

  //*********************************************************************

  //Теперь настройте внутренний БИХ-фильтр.
  // БИХ-фильтр (Infinite Impulse Response) подавляет высокочастотные колебания.
  // Высокое значение коэффициента означает меньше шума, но измерения становятся менее чувствительны.

  //0b000:      factor 0 (filter off)
  //0b001:      factor 2
  //0b010:      factor 4
  //0b011:      factor 8
  //0b100:      factor 16 (default value)

  bme.parameter.IIRfilter = 0b100; //IIR Filter for Sensor

  bme.parameter.humidOversampling = 0b101; //Определяем коэффициент передискретизации для измерений влажности (default value)
  bme.parameter.tempOversampling = 0b101;  // Определяем коэффициент передискретизации для измерений температуры (default value)
  bme.parameter.pressOversampling = 0b101; // Определяем коэффициент передискретизации для измерений давления (default value)

  //*********************************************************************

  // Для точных измерений высоты, пожалуйста, нужно указать текущее давление с поправкой на уровень моря

  bme.parameter.pressureSeaLevel = 1013.25; //default value of 1013.25 hPa

  // Также введите текущую среднюю температуру снаружи (да, действительно снаружи!)
  // Для чуть менее точных измерений высоты просто оставьте стандартную температуру по умолчанию (15 ° C и 59 ° F);

  bme.parameter.tempOutsideCelsius = 15; //default value of 15°C

  bme.parameter.tempOutsideFahrenheit = 59; //default value of 59°F

#endif
}
void setup()
{
  #ifdef DEBUGA
  Serial.begin(9600);

  Serial.println();
  Serial.println("mounting FS...");
  #endif
  settings_read();
  WiFi.macAddress(macb);
  wiffi();

  //reset settings - for testing
  if (strcmp(settingss.name, namer) == 0){//
  strncpy(settingss.name, "12345", sizeof(settingss.name));
  settings_save();
  wiffireset();
  }

  //Wire.begin(D1, D2);
  Wire.begin(D2,D1);
  pinMode(LED, OUTPUT);

  
  snprintf(mac, 120, "ua/%s/%02x%02x%02x%02x%02x%02x",blynk_token, macb[0], macb[1], macb[2], macb[3], macb[4], macb[5]);
  #ifdef DEBUGA
  Serial.print("mac ");
  Serial.println(mac);
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP()); // Send the IP address of the ESP8266 to the computer
  #endif
  digitalWrite(LED, HIGH);
  //update();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  #ifdef DS18B20ON
    ds.begin();
  #endif
  #ifdef CCS811ON
    myCCS811.begin();
  #endif
  #ifdef HTU21DON
    //myHumidity.begin();
    myHTU21D.begin();
  #endif
  #ifdef MHZ19ON
  mhz19.begin(SERIAL_RX, SERIAL_TX);
  mhz19.setAutoCalibration((strcmp(settingss.pass, calibr) == 0));
  /*while ( mhz19.isWarming() ) {
    Serial.print("MH-Z19 now warming up...  status:"); Serial.println(mhz19.getStatus());
    delay(1000);
  }*/
  #endif
  BME280setup();
  
  myTZ.setLocation(F("Europe/Kiev"));
  
}
boolean sendsensors(char *dat, int no, char *what, float val)
{
  if (val>0){
  snprintf(topik, TOPIK_BUFFER_SIZE, "%s/sensors/%s/%d/%sT", mac, dat, no, what);
  String dataaa = myTZ.dateTime();
  dataaa.toCharArray(mdebug, dataaa.length());
  client.publish(topik, mdebug);
  }
  snprintf(topik, TOPIK_BUFFER_SIZE, "%s/sensors/%s/%d/%s", mac, dat, no, what);
  snprintf(msg, MSG_BUFFER_SIZE, "%.1f", val);
  #ifdef DEBUGA
    Serial.println(topik);
    Serial.println(msg);
    //Serial.println(dataaa);
  #endif
  
  return client.publish(topik, msg);
  
}
boolean sendsensormhz( unsigned char response[9])
{
  snprintf(topik, TOPIK_BUFFER_SIZE, "%s/sensors/MHZ19/damp", mac);
  //snprintf(msg, MSG_BUFFER_SIZE, "%s", val);
  snprintf(mdebug, TOPIK_BUFFER_SIZE, "%02x %02x %02x %02x %02x %02x %02x %02x %02x", 
  response[0], response[1], response[2], response[3], response[4], response[5], response[6], response[7], response[8]);
  #ifdef DEBUGA
    Serial.println(topik);
    Serial.println(mdebug);
  #endif
  return client.publish(topik, mdebug); 
}

void MHZ19run()
{
#ifdef MHZ19ON
  //for (int run=0;run<1;run++){
    //sendsensors("MHZ19", 0, "CO2", mhz19.getMeasurement());
    #ifdef DEBUGA
    Serial.print("co2: "); Serial.print(mhz19.getMeasurement().co2_ppm); Serial.println("ppm now.");
    #endif
    sendsensors("MHZ19", 0, "CO2", mhz19.getMeasurement().co2_ppm + settingss.mhz19c[0]);
    sendsensors("MHZ19", 0, "temperature", mhz19.getMeasurement().temperature);
    sendsensors("MHZ19", 0, "state", mhz19.getMeasurement().state);
  //mySerial.write(cmd, 9);
  
  //mySerial.write(CMD_ABC_OFF, 9);
  
  /*delay(100);
  //memset(response, 0, 9);
  //mySerial.readBytes(response, 9);
  //sendsensors("MHZ19", 0, "CO2", (256 * responseHigh) + responseLow + settingss.mhz19c[0]);
  int i;
  byte crc = 0;
  for (i = 1; i < 8; i++) crc += response[i];
  crc = 255 - crc;
  crc++;
  sendsensormhz(response);
  if (!(response[0] == 0xFF && response[1] == 0x86 && response[8] == crc))
  {
    Serial.println(String(run)+" CRC error: " + String(crc) + " / " + String(response[8]));
  }
  else
  {
    unsigned int responseHigh = (unsigned int)response[2];
    unsigned int responseLow = (unsigned int)response[3];
    unsigned int ppm = (256 * responseHigh) + responseLow;
    Serial.println("CO2: " + String(ppm) + " ppm");
    sendsensors("MHZ19", 0, "CO2", ppm + settingss.mhz19c[0]);
    break;
  }*/
  //}
#endif
}
void HTU21()
{
#ifdef HTU21DON
  float humd = myHTU21D.readHumidity();
  float temp = myHTU21D.readTemperature();

  #ifdef DEBUGA
  Serial.print("Time:");
  Serial.print(millis());
  Serial.print(" Temperature:");
  Serial.print(temp, 1);
  Serial.print("C");
  Serial.print(" Humidity:");
  Serial.print(humd, 1);
  Serial.print("%");
  Serial.println();
  #endif
  if (temp!=255.0||humd!=255.0){
        sendsensors("HTU21D", 0, "Humidity", humd+ settingss.htu21dh[0]);
        sendsensors("HTU21D", 0, "Temperature_C", temp+ settingss.htu21dt[0]);
      }
  
#endif
}
void CCS811S()
{
#ifdef CCS811ON
if (myCCS811.dataAvailable()) //Check to see if CCS811 has new data (it's the slowest sensor)
  {
    myCCS811.readAlgorithmResults(); //Read latest from CCS811 and update tVOC and CO2 variables
    //getWeather(); //Get latest humidity/pressure/temp data from BME280
    //printData(); //Pretty print all the data
      #ifdef DEBUGA
      Serial.print("CO2: ");
      Serial.print(myCCS811.getCO2());
      Serial.print("ppm, TVOC: ");
      Serial.println(myCCS811.getTVOC());
      #endif
      
      sendsensors("CCS811", 0, "CO2", myCCS811.getCO2()+ settingss.ccs811c[0]);
      sendsensors("CCS811", 0, "TVOC", myCCS811.getTVOC()+ settingss.ccs811tv[0]);
  }
  #ifdef DEBUGA
  else if (myCCS811.checkForStatusError()) //Check to see if CCS811 has thrown an error
  {
    Serial.print("CSS811 err: ");
    Serial.println(myCCS811.getErrorRegister()); //Prints whatever CSS811 error flags are detected
  }
  #endif

#endif
}
void BME280()
{
#ifdef BME280ON
  if (bme.init() != 0x60)
  {
    #ifdef DEBUGA
    Serial.println("BME280 Sensor not found, check wiring!");
    //delay(1000);
    //while(1);
    #endif
  }
  else
  {
    #ifdef DEBUGA
    Serial.println("BME280 Sensor detected!");
    Serial.print("Temperature [°C]:");
    Serial.println(bme.readTempC());
    Serial.print("Humidity [%]:");
    Serial.println(bme.readHumidity());
    Serial.print("Pressure [hPa]:");
    Serial.println(bme.readPressure());
    Serial.println();
    Serial.println(F("***************************************************"));
    Serial.println();
    #endif
    sendsensors("BME280", 0, "Temperature_C",  bme.readTempC() + settingss.bme280t[0]);
    sendsensors("BME280", 0, "Humidity",  bme.readHumidity() + settingss.bme280h[0]);
    sendsensors("BME280", 0, "Pressure",  bme.readPressure()+ settingss.bme280p[0]);
    //delay(1000);
  }
#endif
}
void DS18B20()
{
#ifdef DS18B20ON
  numb_of_sens = ds.getDeviceCount();
  #ifdef DEBUGA
  Serial.print("Found ");
  Serial.print(numb_of_sens);
  Serial.println(" devices.");
  Serial.println("Printing addresses...");
  for (int i = 0; i < numb_of_sens; i++)
  {
    Serial.print("Sensor ");
    Serial.print(i);
    Serial.print(" : ");
    //ds.getAddress(insideThermometer, i);
    //printAddress(insideThermometer);
  }
  #endif
  ds.requestTemperatures();
  for (int i = 0; i < numb_of_sens; i++)
  {
    float temp = ds.getTempCByIndex(i);
    #ifdef DEBUGA
    Serial.print("Sensor № ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(temp);
    Serial.println("C");
    #endif
    //ds.getAddress(insideThermometer, i);
    sendsensors("DS18B20", i, "Temperature_C",temp + settingss.ds18b20t[i]);
  }
#endif
}
void loop()
{

  if (!client.connected())
  {
    reconnect();
  }
  else
  {
    events();
    client.loop();
    if (foo.Now())
    { // Код здесь выполняется каждый интервал в микросекундах, указанный в конструкторе выше.
      if (stan == 0)
      {
        snprintf(msg, MSG_BUFFER_SIZE, "%d", WiFi.RSSI());     //+ 24.02.20
        snprintf(topik, TOPIK_BUFFER_SIZE, "%s/sys/dBm", mac); //+ 24.02.20
        if (client.publish(topik, msg) == true)
        {
          #ifdef DEBUGA
          Serial.println("to dBm");
          #endif
        }
      }
      else if (stan == 2)
      {
        DS18B20();
        BME280();
        CCS811S();
        HTU21();
        MHZ19run();
      }
      else if (stan == 3)
      {
        snprintf(msg, MSG_BUFFER_SIZE, "%d", WiFi.RSSI());     //+ 24.02.20
        snprintf(topik, TOPIK_BUFFER_SIZE, "%s/sys/dBm", mac); //+ 24.02.20
        if (client.publish(topik, msg) == true)
        {
          #ifdef DEBUGA
          Serial.println("send");
          #endif
          digitalWrite(LED, LOW);
          delay(500);
          digitalWrite(LED, HIGH);
          #ifdef DEEPSLEEEP
          #ifdef DEBUGA
          Serial.println("sleep");
          #endif
          ESP.deepSleep(56e6);
          #endif
          #ifdef DEBUGA
          Serial.println("no sleep");
          #endif
        }
      }
      else if (stan == 20)
      {
        #ifdef DEBUGA
          Serial.println("no sleep!!");
          #endif
          if (strcmp(settingss.name, namer) == 0){//
            //ESP.reset();
            ESP.restart();
          }
        stan = -1;
      }
      stan++;
    }
    //send();
  }
}
