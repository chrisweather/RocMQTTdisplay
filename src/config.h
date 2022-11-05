// Roc-MQTT-Display CONFIGURATION
// Version 1.08 rerelease
// Copyright (c) 2020-2022 Christian Heinrichs. All rights reserved.
// https://github.com/chrisweather/RocMQTTdisplay

#ifndef CONFIG_H
#define CONFIG_H
#include "template.h"         // Roc-MQTT-Display template file
#include <LittleFS.h>         // LittleFS file system https://github.com/esp8266/Arduino/tree/master/libraries/LittleFS
#include <ArduinoJson.h>      // ArduinoJson by Benoît Blanchon https://github.com/bblanchon/ArduinoJson

// File System
#define FORMAT_LITTLEFS_IF_FAILED true

uint8_t TPL = 0;

struct Sec {
  char WIFI_SSID[50];     // 
  char WIFI_PW[50];       // 
  char OTA_PW[50];        // 
  char OTA_HASH[50];      // 
  char MQTT_USER[50];     // 
  char MQTT_PW[50];       // 
};

struct Config {
  const char* VER = "1.08";
// WIFI
  char     WIFI_DEVICENAME[19];    // Unique Controller Device Name for WiFi network
  uint16_t WIFI_RECONDELAY;        // Delay between WiFi reconnection attempts, default = 60000 ms
// OTA
  char     OTA_HOSTNAME[19];       // Unique OTA Controller Hostname, default when empty: esp8266-[ChipID]
  uint16_t OTA_PORT;               // OTA Port, default = 8266
// NTP
  char     NTP_SERVER[51];         // NTP Time Server pool providing UTC time, Europe: 0.europe.pool.ntp.org, Germany: de.pool.ntp.org
  char     NTP_TZ[51];             // NTP Timezone and Daylight Saving Time start/end  https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
// MQTT
  char     MQTT_IP[18];            // MQTT broker IP-adress
  uint16_t MQTT_PORT = 1883;       // MQTT broker Port, default = 1883, currently hardcoded, change here if required
  uint16_t MQTT_MSGSIZE;           // Max. MQTT packet size, default = 128 bytes
  uint16_t MQTT_KEEPALIVE1;        // MQTT keep alive, default = 15 sec, min = 1 sec
  uint16_t MQTT_RECONDELAY;        // Delay between MQTT reconnection attempts, default = 15000 ms
  uint8_t  MQTT_DEBUG;             // Enable MQTT debugging messages sent to serial output, 0=off, 1=on
  char     MQTT_TOPIC1[50];        // MQTT Topic 1, default = "rocrail/service/info/clock"
  char     MQTT_TOPIC2[50];        // MQTT Topic 1, default = "rocrail/service/info/tx"
  char     MQTT_DELIMITER[5];      // MQTT delimiter (e.g. ";" or " , " for message payload, will be replaced by "#" before processing. Default: "#"
// DISPLAYS
  //uint8_t  DISPSIZE = 0;           // 0=128x32, 1=128x64, 2=64x48, 3=96x16, 4=80x160, default = 0
  uint8_t  DISPWIDTH;              // Display width in pixel
  uint8_t  DISPHEIGHT;             // Display height in pixel
  uint8_t  MUX;                    // TCA9548A I2C Multiplexer address, default: 112 (0x70), 0=one display connected without multiplexer
  uint8_t  NUMDISP;                // Number of I2C OLED displays connected to this controller, 1-8
  uint16_t STARTDELAY;             // Show Controllername and Display Number x milliseconds longer at startup, helpful during setup
  uint8_t  UPDSPEED;               // Slow down display update intervall by increasing the number, e.g. 30 = base speed + 30ms
  uint8_t  SCREENSAVER;            // minutes without MQTT message received until screenSaver switches all displays into power save mode, 0=off
  uint8_t  PRINTBUF;               // When 1: Print display buffer of display 1 to serial out as XBM, default: 0
};

// Configuration for displays connected to this controller (Disp) 1-8
//                          Disp1, Disp2, Disp3, Disp4, Disp5, Disp6, Disp7, Disp8
char     DPL_id[8][4] =    { "D01", "D02", "D03", "D04", "D05", "D06", "D07", "D08" };  // ID's of Displays 1-8 connected to this controller, e.g. D01...D99
uint8_t  DPL_flip[] =      {     0,     1,     1,     0,     0,     1,     1,     0 };  // 0,1  180 degree hardware based rotation of the internal frame buffer when 1
//uint8_t  DPL_rotation[] =  {     0,     0,     0,     0,     0,     0,     0,     0 };  // 0,90,180,270  software based display content rotation by 0, 90, 180, 270 degrees, still work in progress
uint8_t  DPL_contrast[] =  {     1,     1,     1,     1,     1,     1,     1,     1 };  // 0-255  0=display off (works with some displays only), default = 1, 255 max brightness, change requires reboot
uint8_t  DPL_side[] =      {     1,     0,     0,     1,     1,     0,     0,     1 };  // 0,1  0=Side A, 1=Side B

struct Template {
};

const char *secfile = "/rmdsec.txt";         // 8.3 filename
Sec sec;                                     // global sec object
const char *configfile = "/rmdcfg.txt";
Config config;
const char *templatefile = "/rmdtpl.txt";
Template templ;

const char *template00 = "/rmdtpl00.txt";
const char *template01 = "/rmdtpl01.txt";
const char *template02 = "/rmdtpl02.txt";
const char *template03 = "/rmdtpl03.txt";
const char *template04 = "/rmdtpl04.txt";
const char *template05 = "/rmdtpl05.txt";
const char *template06 = "/rmdtpl06.txt";
const char *template07 = "/rmdtpl07.txt";
const char *template08 = "/rmdtpl08.txt";
const char *template09 = "/rmdtpl09.txt";


// Load configuration from file
void loadConfiguration(const char *configfile, Config &config)
{
  // Open config json file for reading
  File file = LittleFS.open(configfile, "r");
  delay(200);
  StaticJsonDocument<1800> doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.println(F("Failed to convert json file, using default configuration"));
    Serial.print(F("deserializeJson() returned "));
    Serial.println(error.c_str());
    if (doc.capacity() == 0) {
     Serial.println("allocation failed!");
    }
  }

  // Copy values from JsonDocument to Config, use defaults in case file is not readable
  strlcpy(config.WIFI_DEVICENAME, doc["WIFI_DEVICENAME"] | "RocMQTTdisplayC01", sizeof(config.WIFI_DEVICENAME));
  config.WIFI_RECONDELAY = doc["WIFI_RECONDELAY"] | 5000;
  strlcpy(config.OTA_HOSTNAME, doc["OTA_HOSTNAME"] | "RocMQTTdisplayC01", sizeof(config.OTA_HOSTNAME));
  config.OTA_PORT = doc["OTA_PORT"] | 8266;
  strlcpy(config.NTP_SERVER, doc["NTP_SERVER"] | "0.europe.pool.ntp.org", sizeof(config.NTP_SERVER));
  strlcpy(config.NTP_TZ, doc["NTP_TZ"] | "CET-1CEST,M3.5.0,M10.5.0/3", sizeof(config.NTP_TZ));
  strlcpy(config.MQTT_IP, doc["MQTT_IP"] | "", sizeof(config.MQTT_IP));
  config.MQTT_PORT = doc["MQTT_PORT"] | 1883;
  config.MQTT_MSGSIZE = doc["MQTT_MSGSIZE"] | 256;
  config.MQTT_KEEPALIVE1 = doc["MQTT_KEEPALIVE"] | 15;
  config.MQTT_RECONDELAY = doc["MQTT_RECONDELAY"] | 10000;
  strlcpy(config.MQTT_TOPIC1, doc["MQTT_TOPIC1"] | "rocrail/service/info/clock", sizeof(config.MQTT_TOPIC1));
  strlcpy(config.MQTT_TOPIC2, doc["MQTT_TOPIC2"] | "rocrail/service/info/tx", sizeof(config.MQTT_TOPIC2));
  strlcpy(config.MQTT_DELIMITER, doc["MQTT_DELIMITER"] | "", sizeof(config.MQTT_DELIMITER));
  config.MQTT_DEBUG = doc["MQTT_DEBUG"] | 0;
  config.MUX = doc["MUX"] | 112;
  config.NUMDISP = doc["NUMDISP"] | 2;
  config.DISPWIDTH = doc["DISPWIDTH"] | 128;
  config.DISPHEIGHT = doc["DISPHEIGHT"] | 32;
  config.STARTDELAY = doc["STARTDELAY"] | 200;
  config.UPDSPEED = doc["UPDSPEED"] | 0;
  config.SCREENSAVER = doc["SCREENSAVER"] | 60;
  config.PRINTBUF = doc["PRINTBUF"] | 0;
  strlcpy(DPL_id[0], doc["DPL_ID0"] | "D01", sizeof(DPL_id[0]));
  strlcpy(DPL_id[1], doc["DPL_ID1"] | "D02", sizeof(DPL_id[1]));
  strlcpy(DPL_id[2], doc["DPL_ID2"] | "D03", sizeof(DPL_id[2]));
  strlcpy(DPL_id[3], doc["DPL_ID3"] | "D04", sizeof(DPL_id[3]));
  strlcpy(DPL_id[4], doc["DPL_ID4"] | "D05", sizeof(DPL_id[4]));
  strlcpy(DPL_id[5], doc["DPL_ID5"] | "D06", sizeof(DPL_id[5]));
  strlcpy(DPL_id[6], doc["DPL_ID6"] | "D07", sizeof(DPL_id[6]));
  strlcpy(DPL_id[7], doc["DPL_ID7"] | "D08", sizeof(DPL_id[7]));
  DPL_flip[0] = doc["DPL_FLIP0"] | 0;
  DPL_flip[1] = doc["DPL_FLIP1"] | 0;
  DPL_flip[2] = doc["DPL_FLIP2"] | 0;
  DPL_flip[3] = doc["DPL_FLIP3"] | 0;
  DPL_flip[4] = doc["DPL_FLIP4"] | 0;
  DPL_flip[5] = doc["DPL_FLIP5"] | 0;
  DPL_flip[6] = doc["DPL_FLIP6"] | 0;
  DPL_flip[7] = doc["DPL_FLIP7"] | 0;
  DPL_contrast[0] = doc["DPL_CONTRAST0"] | 1;
  DPL_contrast[1] = doc["DPL_CONTRAST1"] | 1;
  DPL_contrast[2] = doc["DPL_CONTRAST2"] | 1;
  DPL_contrast[3] = doc["DPL_CONTRAST3"] | 1;
  DPL_contrast[4] = doc["DPL_CONTRAST4"] | 1;
  DPL_contrast[5] = doc["DPL_CONTRAST5"] | 1;
  DPL_contrast[6] = doc["DPL_CONTRAST6"] | 1;
  DPL_contrast[7] = doc["DPL_CONTRAST7"] | 1;
  DPL_side[0] = doc["DPL_SIDE0"] | 0;
  DPL_side[1] = doc["DPL_SIDE1"] | 0;
  DPL_side[2] = doc["DPL_SIDE2"] | 0;
  DPL_side[3] = doc["DPL_SIDE3"] | 0;
  DPL_side[4] = doc["DPL_SIDE4"] | 0;
  DPL_side[5] = doc["DPL_SIDE5"] | 0;
  DPL_side[6] = doc["DPL_SIDE6"] | 0;
  DPL_side[7] = doc["DPL_SIDE7"] | 0;

  file.close();
}

// Save configuration to a file
void saveConfiguration(const char *configfile, const Config &config) 
{
  // Delete existing file, otherwise the configuration will be appended to the file
  LittleFS.remove(configfile);

  // Open file for writing
  File file = LittleFS.open(configfile, "w");
  if (!file) {
    Serial.println(F("Failed to create file"));
    return;
  }

  StaticJsonDocument<1800> doc;
  doc["VER"] = config.VER;
  doc["WIFI_DEVICENAME"] = config.WIFI_DEVICENAME;
  doc["WIFI_RECONDELAY"] = config.WIFI_RECONDELAY;
  doc["OTA_HOSTNAME"] = config.OTA_HOSTNAME;
  doc["OTA_PORT"] = config.OTA_PORT;
  doc["NTP_SERVER"] = config.NTP_SERVER;
  doc["NTP_TZ"] = config.NTP_TZ;
  doc["MQTT_IP"] = config.MQTT_IP;
  doc["MQTT_PORT"] = config.MQTT_PORT;
  doc["MQTT_MSGSIZE"] = config.MQTT_MSGSIZE;
  doc["MQTT_KEEPALIVE"] = config.MQTT_KEEPALIVE1;
  doc["MQTT_RECONDELAY"] = config.MQTT_RECONDELAY;
  doc["MQTT_TOPIC1"] = config.MQTT_TOPIC1;
  doc["MQTT_TOPIC2"] = config.MQTT_TOPIC2;
  doc["MQTT_DELIMITER"] = config.MQTT_DELIMITER;
  doc["MQTT_DEBUG"] = config.MQTT_DEBUG;
  doc["MUX"] = config.MUX;
  doc["NUMDISP"] = config.NUMDISP;
  doc["DISPWIDTH"] = config.DISPWIDTH;
  doc["DISPHEIGHT"] = config.DISPHEIGHT;
  doc["STARTDELAY"] = config.STARTDELAY;
  doc["UPDSPEED"] = config.UPDSPEED;
  doc["SCREENSAVER"] = config.SCREENSAVER;
  doc["DPL_ID0"] = DPL_id[0];
  doc["DPL_ID1"] = DPL_id[1];
  doc["DPL_ID2"] = DPL_id[2];
  doc["DPL_ID3"] = DPL_id[3];
  doc["DPL_ID4"] = DPL_id[4];
  doc["DPL_ID5"] = DPL_id[5];
  doc["DPL_ID6"] = DPL_id[6];
  doc["DPL_ID7"] = DPL_id[7];
  doc["DPL_FLIP0"] = DPL_flip[0];
  doc["DPL_FLIP1"] = DPL_flip[1];
  doc["DPL_FLIP2"] = DPL_flip[2];
  doc["DPL_FLIP3"] = DPL_flip[3];
  doc["DPL_FLIP4"] = DPL_flip[4];
  doc["DPL_FLIP5"] = DPL_flip[5];
  doc["DPL_FLIP6"] = DPL_flip[6];
  doc["DPL_FLIP7"] = DPL_flip[7];
  doc["DPL_CONTRAST0"] = DPL_contrast[0];
  doc["DPL_CONTRAST1"] = DPL_contrast[1];
  doc["DPL_CONTRAST2"] = DPL_contrast[2];
  doc["DPL_CONTRAST3"] = DPL_contrast[3];
  doc["DPL_CONTRAST4"] = DPL_contrast[4];
  doc["DPL_CONTRAST5"] = DPL_contrast[5];
  doc["DPL_CONTRAST6"] = DPL_contrast[6];
  doc["DPL_CONTRAST7"] = DPL_contrast[7];
  doc["DPL_SIDE0"] = DPL_side[0];
  doc["DPL_SIDE1"] = DPL_side[1];
  doc["DPL_SIDE2"] = DPL_side[2];
  doc["DPL_SIDE3"] = DPL_side[3];
  doc["DPL_SIDE4"] = DPL_side[4];
  doc["DPL_SIDE5"] = DPL_side[5];
  doc["DPL_SIDE6"] = DPL_side[6];
  doc["DPL_SIDE7"] = DPL_side[7];

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write json to file"));
  }
  file.close();
  delay(1000);
}


// Load template data from file - fonts, logos
//void loadTemplate(const char *templatefile, Template &templ)
void loadTemplate(const char *templatefile)
{
  // Open template data json file for reading
  File file = LittleFS.open(templatefile, "r");
  delay(200);
  StaticJsonDocument<1800> doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.println(F("Failed to convert json file, using default configuration"));
    Serial.print(F("deserializeJson() returned "));
    Serial.println(error.c_str());
    if (doc.capacity() == 0) {
     Serial.println("allocation failed!");
    }
  }
  else {
  // Copy values from JsonDocument to templ, use defaults in case file is not readable
  strlcpy(logoId[0], doc["LOGOID0"] | "", sizeof(logoId[0]));
  strlcpy(logoId[1], doc["LOGOID1"] | "", sizeof(logoId[1]));
  strlcpy(logoId[2], doc["LOGOID2"] | "", sizeof(logoId[2]));
  strlcpy(logoId[3], doc["LOGOID3"] | "", sizeof(logoId[3]));
  strlcpy(logoId[4], doc["LOGOID4"] | "", sizeof(logoId[4]));
  strlcpy(logoId[5], doc["LOGOID5"] | "", sizeof(logoId[5]));
  strlcpy(logoId[6], doc["LOGOID6"] | "", sizeof(logoId[6]));
  strlcpy(logoId[7], doc["LOGOID7"] | "", sizeof(logoId[7]));
  strlcpy(logoId[8], doc["LOGOID8"] | "", sizeof(logoId[8]));
  strlcpy(logoId[9], doc["LOGOID9"] | "", sizeof(logoId[9]));
  strlcpy(logoId[10], doc["LOGOID10"] | "", sizeof(logoId[10]));
  strlcpy(logoId[11], doc["LOGOID11"] | "", sizeof(logoId[11]));
  strlcpy(logoId[12], doc["LOGOID12"] | "", sizeof(logoId[12]));
  strlcpy(logoId[13], doc["LOGOID13"] | "", sizeof(logoId[13]));
  strlcpy(logoId[14], doc["LOGOID14"] | "", sizeof(logoId[14]));
  strlcpy(logoId[15], doc["LOGOID15"] | "", sizeof(logoId[15]));
  strlcpy(logoId[16], doc["LOGOID16"] | "", sizeof(logoId[16]));
  strlcpy(logoId[17], doc["LOGOID17"] | "", sizeof(logoId[17]));
  strlcpy(logoId[18], doc["LOGOID18"] | "", sizeof(logoId[18]));
  strlcpy(logoId[19], doc["LOGOID19"] | "", sizeof(logoId[19]));

  logow[0] = doc["LOGO0W"] | 10;
  logow[1] = doc["LOGO1W"] | 10;
  logow[2] = doc["LOGO2W"] | 10;
  logow[3] = doc["LOGO3W"] | 10;
  logow[4] = doc["LOGO4W"] | 10;
  logow[5] = doc["LOGO5W"] | 10;
  logow[6] = doc["LOGO6W"] | 10;
  logow[7] = doc["LOGO7W"] | 10;
  logow[8] = doc["LOGO8W"] | 10;
  logow[9] = doc["LOGO9W"] | 10;
  logow[10] = doc["LOGO10W"] | 10;
  logow[11] = doc["LOGO11W"] | 10;
  logow[12] = doc["LOGO12W"] | 10;
  logow[13] = doc["LOGO13W"] | 10;
  logow[14] = doc["LOGO14W"] | 10;
  logow[15] = doc["LOGO15W"] | 10;
  logow[16] = doc["LOGO16W"] | 10;
  logow[17] = doc["LOGO17W"] | 10;
  logow[18] = doc["LOGO18W"] | 10;
  logow[19] = doc["LOGO19W"] | 10;

  logoh[0] = doc["LOGO0H"] | 10;
  logoh[1] = doc["LOGO1H"] | 10;
  logoh[2] = doc["LOGO2H"] | 10;
  logoh[3] = doc["LOGO3H"] | 10;
  logoh[4] = doc["LOGO4H"] | 10;
  logoh[5] = doc["LOGO5H"] | 10;
  logoh[6] = doc["LOGO6H"] | 10;
  logoh[7] = doc["LOGO7H"] | 10;
  logoh[8] = doc["LOGO8H"] | 10;
  logoh[9] = doc["LOGO9H"] | 10;
  logoh[10] = doc["LOGO10H"] | 10;
  logoh[11] = doc["LOGO11H"] | 10;
  logoh[12] = doc["LOGO12H"] | 10;
  logoh[13] = doc["LOGO13H"] | 10;
  logoh[14] = doc["LOGO14H"] | 10;
  logoh[15] = doc["LOGO15H"] | 10;
  logoh[16] = doc["LOGO16H"] | 10;
  logoh[17] = doc["LOGO17H"] | 10;
  logoh[18] = doc["LOGO18H"] | 10;
  logoh[19] = doc["LOGO19H"] | 10;
/*
  strlcpy(logo0, doc["LOGO0"] | "", sizeof(logo0));
  strlcpy(logo1, doc["LOGO1"] | "", sizeof(logo1));
  strlcpy(logo2, doc["LOGO2"] | "", sizeof(logo2));
  strlcpy(logo3, doc["LOGO3"] | "", sizeof(logo3));
  strlcpy(logo4, doc["LOGO4"] | "", sizeof(logo4));
  strlcpy(logo5, doc["LOGO5"] | "", sizeof(logo5));
  strlcpy(logo6, doc["LOGO6"] | "", sizeof(logo6));
  strlcpy(logo7, doc["LOGO7"] | "", sizeof(logo7));
  strlcpy(logo8, doc["LOGO8"] | "", sizeof(logo8));
  strlcpy(logo9, doc["LOGO9"] | "", sizeof(logo9));
*/
  file.close();
  }
}


// Save template data to a file - fonts, logos
//void saveTemplate(const char *templatefile, const Template &templ)
void saveTemplate(const char *templatefile)
{
  // Delete existing file, otherwise the template data will be appended to the file
  LittleFS.remove(templatefile);

  // Open file for writing
  File file = LittleFS.open(templatefile, "w");
  if (!file) {
    Serial.println(F("Failed to create file"));
    return;
  }

  StaticJsonDocument<1800> doc;
  doc["LOGOID0"] = logoId[0];
  doc["LOGOID1"] = logoId[1];
  doc["LOGOID2"] = logoId[2];
  doc["LOGOID3"] = logoId[3];
  doc["LOGOID4"] = logoId[4];
  doc["LOGOID5"] = logoId[5];
  doc["LOGOID6"] = logoId[6];
  doc["LOGOID7"] = logoId[7];
  doc["LOGOID8"] = logoId[8];
  doc["LOGOID9"] = logoId[9];
  doc["LOGOID10"] = logoId[10];
  doc["LOGOID11"] = logoId[11];
  doc["LOGOID12"] = logoId[12];
  doc["LOGOID13"] = logoId[13];
  doc["LOGOID14"] = logoId[14];
  doc["LOGOID15"] = logoId[15];
  doc["LOGOID16"] = logoId[16];
  doc["LOGOID17"] = logoId[17];
  doc["LOGOID18"] = logoId[18];
  doc["LOGOID19"] = logoId[19];
  
  doc["LOGO0W"] = logow[0];
  doc["LOGO1W"] = logow[1];
  doc["LOGO2W"] = logow[2];
  doc["LOGO3W"] = logow[3];
  doc["LOGO4W"] = logow[4];
  doc["LOGO5W"] = logow[5];
  doc["LOGO6W"] = logow[6];
  doc["LOGO7W"] = logow[7];
  doc["LOGO8W"] = logow[8];
  doc["LOGO9W"] = logow[9];
  doc["LOGO10W"] = logow[10];
  doc["LOGO11W"] = logow[11];
  doc["LOGO12W"] = logow[12];
  doc["LOGO13W"] = logow[13];
  doc["LOGO14W"] = logow[14];
  doc["LOGO15W"] = logow[15];
  doc["LOGO16W"] = logow[16];
  doc["LOGO17W"] = logow[17];
  doc["LOGO18W"] = logow[18];
  doc["LOGO19W"] = logow[19];
  
  doc["LOGO0H"] = logoh[0];
  doc["LOGO1H"] = logoh[1];
  doc["LOGO2H"] = logoh[2];
  doc["LOGO3H"] = logoh[3];
  doc["LOGO4H"] = logoh[4];
  doc["LOGO5H"] = logoh[5];
  doc["LOGO6H"] = logoh[6];
  doc["LOGO7H"] = logoh[7];
  doc["LOGO8H"] = logoh[8];
  doc["LOGO9H"] = logoh[9];
  doc["LOGO10H"] = logoh[10];
  doc["LOGO11H"] = logoh[11];
  doc["LOGO12H"] = logoh[12];
  doc["LOGO13H"] = logoh[13];
  doc["LOGO14H"] = logoh[14];
  doc["LOGO15H"] = logoh[15];
  doc["LOGO16H"] = logoh[16];
  doc["LOGO17H"] = logoh[17];
  doc["LOGO18H"] = logoh[18];
  doc["LOGO19H"] = logoh[19];
/*
  String lbuf = "";
  for (uint8_t i = 0; i < (logo0size); i++){
    if (uint8_t(logo0[i]) < 16){
      lbuf += String("0x0");
    }
    else {
      lbuf += String("0x");
    }
    lbuf += String(logo0[i], HEX);
    if (i < (logo0size - 1)) lbuf += String(", ");
  }
  doc["LOGO0"] = lbuf;

  doc["LOGO1"] = logo1;
  doc["LOGO2"] = logo2;
  doc["LOGO3"] = logo3;
  doc["LOGO4"] = logo4;
  doc["LOGO5"] = logo5;
  doc["LOGO6"] = logo6;
  doc["LOGO7"] = logo7;
  doc["LOGO8"] = logo8;
  doc["LOGO9"] = logo9;
*/
  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write json to file"));
  }
  file.close();
  delay(1000);
}


// Load templates T0-T9 from files
void loadTemplateFile(const char *templatexx)
{
  // Open config json file for reading
  File file = LittleFS.open(templatexx, "r");
  if (!file) {
    Serial.println("file open failed");
    //delay(300);
    unsigned long tn = 0;
    if(millis() > tn + 500){
      tn = millis();
    }
  }

  StaticJsonDocument<1600> doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.println(F("Failed to convert json file, using default configuration"));
    Serial.print(F("deserializeJson() returned "));
    Serial.println(error.c_str());
    if (doc.capacity() == 0) {
     Serial.println("allocation failed!");
    }
  }
  else {
  // Copy values from JsonDocument to Config, use defaults in case file is not readable
  strlcpy(TPL_id[TPL], doc["TPLID"] | "T", sizeof(TPL_id[TPL]));
  strlcpy(TPL_name[TPL], doc["TPLNAME"] | "", sizeof(TPL_name[TPL]));
  TPL_side[TPL] = doc["TPLSIDE"] | 0;
  TPL_invert[TPL] = doc["TPLINV"] | 0;
// Field 0 - Station
  TPL_0font[TPL] = doc["TPL0FONT"] | 2;
  TPL_0maxwidth[TPL] = doc["TPL0MAXWIDTH"] | 116;
  TPL_0font2[TPL] = doc["TPL0FONT2"] | 3;
  TPL_0drawcolor[TPL] = doc["TPL0DRAWCOLOR"] | 1;
  TPL_0fontmode[TPL] = doc["TPL0FONTMODE"] | 1;
  TPL_0posx[TPL] = doc["TPL0POSX"] | 0;
  TPL_0posy[TPL] = doc["TPL0POSY"] | 0;
  TPL_0scroll[TPL] = doc["TPL0SCROLL"] | 0;
// Field 1 - Track
  TPL_1font[TPL] = doc["TPL1FONT"] | 1;
  TPL_1drawcolor[TPL] = doc["TPL1DRAWCOLOR"] | 1;
  TPL_1fontmode[TPL] = doc["TPL1FONTMODE"] | 1;
  TPL_1posx[TPL] = doc["TPL1POSX"] | 0;
  TPL_1posy[TPL] = doc["TPL1POSY"] | 30;
// Field 2 - Destination
  TPL_2font[TPL] = doc["TPL2FONT"] | 2;
  TPL_2maxwidth[TPL] = doc["TPL2MAXWIDTH"] | 116;
  TPL_2font2[TPL] = doc["TPL2FONT2"] | 3;
  TPL_2drawcolor[TPL] = doc["TPL2DRAWCOLOR"] | 1;
  TPL_2fontmode[TPL] = doc["TPL2FONTMODE"] | 1;
  TPL_2posx[TPL] = doc["TPL2POSX"] | 20;
  TPL_2posy[TPL] = doc["TPL2POSY"] | 29;
  TPL_2scroll[TPL] = doc["TPL2SCROLL"] | 0;
// Field 3 - Departure
  TPL_3font[TPL] = doc["TPL3FONT"] | 4;
  TPL_3drawcolor[TPL] = doc["TPL3DRAWCOLOR"] | 1;
  TPL_3fontmode[TPL] = doc["TPL3FONTMODE"] | 1;
  TPL_3posx[TPL] = doc["TPL3POSX"] | 100;
  TPL_3posy[TPL] = doc["TPL3POSY"] | 8;
// Field 4 - Train
  TPL_4font[TPL] = doc["TPL4FONT"] | 5;
  TPL_4drawcolor[TPL] = doc["TPL4DRAWCOLOR"] | 1;
  TPL_4fontmode[TPL] = doc["TPL4FONTMODE"] | 1;
  TPL_4posx[TPL] = doc["TPL4POSX"] | 93;
  TPL_4posy[TPL] = doc["TPL4POSY"] | 17;
// Field 5 - Train Type
  TPL_5logox[TPL] = doc["TPL5LOGOX"] | 0;
  TPL_5logoy[TPL] = doc["TPL5LOGOY"] | 0;
// Field 6 - Message
  TPL_6font[TPL] = doc["TPL6FONT"] | 6;
  TPL_6maxwidth[TPL] = doc["TPL6MAXWIDTH"] | 116;
  TPL_6font2[TPL] = doc["TPL6FONT2"] | 1;
  TPL_6drawcolor[TPL] = doc["TPL6DRAWCOLOR"] | 1;
  TPL_6fontmode[TPL] = doc["TPL6FONTMODE"] | 0;
  TPL_6posx[TPL] = doc["TPL6POSX"] | 0;
  TPL_6posy[TPL] = doc["TPL6POSY"] | 8;
  TPL_6scroll[TPL] = doc["TPL6SCROLL"] | 1;
// Field 6 - Message Scrollbox
  TPL_6boxx[TPL] = doc["TPL6BOXX"] | 20;
  TPL_6boxy[TPL] = doc["TPL6BOXY"] | 0;
  TPL_6boxw[TPL] = doc["TPL6BOXW"] | 90;
  TPL_6boxh[TPL] = doc["TPL6BOXH"] | 10;
// Field 6 - Message Blackbox
  TPL_6drawcolor2[TPL] = doc["TPL6DRAWCOLOR2"] | 0;
  TPL_6fontmode2[TPL] = doc["TPL6FONTMODE2"] | 1;
  TPL_6box2x[TPL] = doc["TPL6BOX2X"] | 91;
  TPL_6box2y[TPL] = doc["TPL6BOX2Y"] | 0;
  TPL_6box2w[TPL] = doc["TPL6BOX2W"] | 127;
  TPL_6box2h[TPL] = doc["TPL6BOX2H"] | 10;
  unsigned long tn = 0;
  if(millis() > tn + 1000){
    tn = millis();
  }
  file.close();
  }
}


// Import template
void importTemplateFile(char *tcontent)
{
  StaticJsonDocument<1600> doc;
  DeserializationError error = deserializeJson(doc, tcontent);
  if (error) {
    Serial.println(F("Failed to convert json import, using default configuration"));
    Serial.print(F("deserializeJson() returned "));
    Serial.println(error.c_str());
    if (doc.capacity() == 0) {
      Serial.println("allocation failed!");
    }
  }
  else {
  // Copy values from JsonDocument to Config, use defaults in case file is not readable
  strlcpy(TPL_id[TPL], doc["TPLID"] | "T", sizeof(TPL_id[TPL]));
  strlcpy(TPL_name[TPL], doc["TPLNAME"] | "Template", sizeof(TPL_name[TPL]));
  TPL_side[TPL] = doc["TPLSIDE"] | 0;
  TPL_invert[TPL] = doc["TPLINV"] | 0;
// Field 0 - Station
  TPL_0font[TPL] = doc["TPL0FONT"] | 2;
  TPL_0maxwidth[TPL] = doc["TPL0MAXWIDTH"] | 116;
  TPL_0font2[TPL] = doc["TPL0FONT2"] | 3;
  TPL_0drawcolor[TPL] = doc["TPL0DRAWCOLOR"] | 1;
  TPL_0fontmode[TPL] = doc["TPL0FONTMODE"] | 1;
  TPL_0posx[TPL] = doc["TPL0POSX"] | 0;
  TPL_0posy[TPL] = doc["TPL0POSY"] | 0;
  TPL_0scroll[TPL] = doc["TPL0SCROLL"] | 0;
// Field 1 - Track
  TPL_1font[TPL] = doc["TPL1FONT"] | 1;
  TPL_1drawcolor[TPL] = doc["TPL1DRAWCOLOR"] | 1;
  TPL_1fontmode[TPL] = doc["TPL1FONTMODE"] | 1;
  TPL_1posx[TPL] = doc["TPL1POSX"] | 0;
  TPL_1posy[TPL] = doc["TPL1POSY"] | 30;
// Field 2 - Destination
  TPL_2font[TPL] = doc["TPL2FONT"] | 2;
  TPL_2maxwidth[TPL] = doc["TPL2MAXWIDTH"] | 116;
  TPL_2font2[TPL] = doc["TPL2FONT2"] | 3;
  TPL_2drawcolor[TPL] = doc["TPL2DRAWCOLOR"] | 1;
  TPL_2fontmode[TPL] = doc["TPL2FONTMODE"] | 1;
  TPL_2posx[TPL] = doc["TPL2POSX"] | 20;
  TPL_2posy[TPL] = doc["TPL2POSY"] | 29;
  TPL_2scroll[TPL] = doc["TPL2SCROLL"] | 0;
// Field 3 - Departure
  TPL_3font[TPL] = doc["TPL3FONT"] | 4;
  TPL_3drawcolor[TPL] = doc["TPL3DRAWCOLOR"] | 1;
  TPL_3fontmode[TPL] = doc["TPL3FONTMODE"] | 1;
  TPL_3posx[TPL] = doc["TPL3POSX"] | 100;
  TPL_3posy[TPL] = doc["TPL3POSY"] | 8;
// Field 4 - Train
  TPL_4font[TPL] = doc["TPL4FONT"] | 5;
  TPL_4drawcolor[TPL] = doc["TPL4DRAWCOLOR"] | 1;
  TPL_4fontmode[TPL] = doc["TPL4FONTMODE"] | 1;
  TPL_4posx[TPL] = doc["TPL4POSX"] | 93;
  TPL_4posy[TPL] = doc["TPL4POSY"] | 17;
// Field 5 - Train Type
  TPL_5logox[TPL] = doc["TPL5LOGOX"] | 0;
  TPL_5logoy[TPL] = doc["TPL5LOGOY"] | 0;
// Field 6 - Message
  TPL_6font[TPL] = doc["TPL6FONT"] | 6;
  TPL_6maxwidth[TPL] = doc["TPL6MAXWIDTH"] | 116;
  TPL_6font2[TPL] = doc["TPL6FONT2"] | 1;
  TPL_6drawcolor[TPL] = doc["TPL6DRAWCOLOR"] | 1;
  TPL_6fontmode[TPL] = doc["TPL6FONTMODE"] | 0;
  TPL_6posx[TPL] = doc["TPL6POSX"] | 0;
  TPL_6posy[TPL] = doc["TPL6POSY"] | 8;
  TPL_6scroll[TPL] = doc["TPL6SCROLL"] | 1;
// Field 6 - Message Scrollbox
  TPL_6boxx[TPL] = doc["TPL6BOXX"] | 20;
  TPL_6boxy[TPL] = doc["TPL6BOXY"] | 0;
  TPL_6boxw[TPL] = doc["TPL6BOXW"] | 90;
  TPL_6boxh[TPL] = doc["TPL6BOXH"] | 10;
// Field 6 - Message Blackbox
  TPL_6drawcolor2[TPL] = doc["TPL6DRAWCOLOR2"] | 0;
  TPL_6fontmode2[TPL] = doc["TPL6FONTMODE2"] | 1;
  TPL_6box2x[TPL] = doc["TPL6BOX2X"] | 91;
  TPL_6box2y[TPL] = doc["TPL6BOX2Y"] | 0;
  TPL_6box2w[TPL] = doc["TPL6BOX2W"] | 127;
  TPL_6box2h[TPL] = doc["TPL6BOX2H"] | 10;
  }
}


// Save templates to files
void saveTemplateFile(const char *templatexx) 
{
  // Delete existing file, otherwise the template data will be appended to the file
  LittleFS.remove(templatexx);

  // Open file for writing
  File file = LittleFS.open(templatexx, "w");
  if (!file) {
    Serial.println(F("Failed to create file"));
    return;
  }

  StaticJsonDocument<1600> doc;
  doc["TPLID"] = TPL_id[TPL];
  doc["TPLNAME"] = TPL_name[TPL];
  doc["TPLSIDE"] = TPL_side[TPL];
  doc["TPLINV"] = TPL_invert[TPL];
// Field 0 - Station
  doc["TPL0FONT"] = TPL_0font[TPL];
  doc["TPL0MAXWIDTH"] = TPL_0maxwidth[TPL];
  doc["TPL0FONT2"] = TPL_0font2[TPL];
  doc["TPL0DRAWCOLOR"] = TPL_0drawcolor[TPL];
  doc["TPL0FONTMODE"] = TPL_0fontmode[TPL];
  doc["TPL0POSX"] = TPL_0posx[TPL];
  doc["TPL0POSY"] = TPL_0posy[TPL];
  doc["TPL0SCROLL"] = TPL_0scroll[TPL];
// Field 1 - Track
  doc["TPL1FONT"] = TPL_1font[TPL];
  doc["TPL1DRAWCOLOR"] = TPL_1drawcolor[TPL];
  doc["TPL1FONTMODE"] = TPL_1fontmode[TPL];
  doc["TPL1POSX"] = TPL_1posx[TPL];
  doc["TPL1POSY"] = TPL_1posy[TPL];
// Field 2 - Destination
  doc["TPL2FONT"] = TPL_2font[TPL];
  doc["TPL2MAXWIDTH"] = TPL_2maxwidth[TPL];
  doc["TPL2FONT2"] = TPL_2font2[TPL];
  doc["TPL2DRAWCOLOR"] = TPL_2drawcolor[TPL];
  doc["TPL2FONTMODE"] = TPL_2fontmode[TPL];
  doc["TPL2POSX"] = TPL_2posx[TPL];
  doc["TPL2POSY"] = TPL_2posy[TPL];
  doc["TPL2SCROLL"] = TPL_2scroll[TPL];
// Field 3 - Departure
  doc["TPL3FONT"] = TPL_3font[TPL];
  doc["TPL3DRAWCOLOR"] = TPL_3drawcolor[TPL];
  doc["TPL3FONTMODE"] = TPL_3fontmode[TPL];
  doc["TPL3POSX"] = TPL_3posx[TPL];
  doc["TPL3POSY"] = TPL_3posy[TPL];
// Field 4 - Train
  doc["TPL4FONT"] = TPL_4font[TPL];
  doc["TPL4DRAWCOLOR"] = TPL_4drawcolor[TPL];
  doc["TPL4FONTMODE"] = TPL_4fontmode[TPL];
  doc["TPL4POSX"] = TPL_4posx[TPL];
  doc["TPL4POSY"] = TPL_4posy[TPL];
// Field 5 - Train Type
  doc["TPL5LOGOX"] = TPL_5logox[TPL];
  doc["TPL5LOGOY"] = TPL_5logoy[TPL];
// Field 6 - Message
  doc["TPL6FONT"] = TPL_6font[TPL];
  doc["TPL6MAXWIDTH"] = TPL_6maxwidth[TPL];
  doc["TPL6FONT2"] = TPL_6font2[TPL];
  doc["TPL6DRAWCOLOR"] = TPL_6drawcolor[TPL];
  doc["TPL6FONTMODE"] = TPL_6fontmode[TPL];
  doc["TPL6POSX"] = TPL_6posx[TPL];
  doc["TPL6POSY"] = TPL_6posy[TPL];
  doc["TPL6SCROLL"] = TPL_6scroll[TPL];
// Field 6 - Message Scrollbox
  doc["TPL6BOXX"] = TPL_6boxx[TPL];
  doc["TPL6BOXY"] = TPL_6boxy[TPL];
  doc["TPL6BOXW"] = TPL_6boxw[TPL];
  doc["TPL6BOXH"] = TPL_6boxh[TPL];
// Field 6 - Message Blackbox
  doc["TPL6DRAWCOLOR2"] = TPL_6drawcolor2[TPL];
  doc["TPL6FONTMODE2"] = TPL_6fontmode2[TPL];
  doc["TPL6BOX2X"] = TPL_6box2x[TPL];
  doc["TPL6BOX2Y"] = TPL_6box2y[TPL];
  doc["TPL6BOX2W"] = TPL_6box2w[TPL];
  doc["TPL6BOX2H"] = TPL_6box2h[TPL];

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write json to file"));
  }
  file.close();
  //delay(2000);
  unsigned long tn = 0;
  if(millis() > tn + 3000){
    tn = millis();
  }
}


// Load sec from file
void loadSecData(const char *secfile, Sec &sec)
{
  // Open sec json file for reading
  File file = LittleFS.open(secfile, "r");
  delay(200);
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.println(F("Failed to convert json file, using default configuration"));
    Serial.print(F("deserializeJson() returned "));
    Serial.println(error.c_str());
    if (doc.capacity() == 0) {
     Serial.println("allocation failed!");
    }
  }
  // Copy values from JsonDocument to Sec, use defaults in case file is not readable
  strlcpy(sec.WIFI_SSID, doc["WIFI_SSID"] | "", sizeof(sec.WIFI_SSID));
  strlcpy(sec.WIFI_PW, doc["WIFI_PW"] | "", sizeof(sec.WIFI_PW));
  strlcpy(sec.OTA_PW, doc["OTA_PW"] | "", sizeof(sec.OTA_PW));
  strlcpy(sec.OTA_HASH, doc["OTA_HASH"] | "", sizeof(sec.OTA_HASH));
  strlcpy(sec.MQTT_USER, doc["MQTT_USER"] | "", sizeof(sec.MQTT_USER));
  strlcpy(sec.MQTT_PW, doc["MQTT_PW"] | "", sizeof(sec.MQTT_PW));
  file.close();
}

// Save sec to a file
void saveSec(const char *secfile, const Sec &sec) 
{
  // Delete existing file, otherwise the sec will be appended to the file
  LittleFS.remove(secfile);

  // Open file for writing
  File file = LittleFS.open(secfile, "w");
  if (!file) {
    Serial.println(F("Failed to create sec file"));
    return;
  }
  StaticJsonDocument<1024> doc;
  doc["WIFI_SSID"] = sec.WIFI_SSID;
  doc["WIFI_PW"] = sec.WIFI_PW;
  doc["OTA_PW"] = sec.OTA_PW;
  doc["OTA_HASH"] = sec.OTA_HASH;
  doc["MQTT_USER"] = sec.MQTT_USER;
  doc["MQTT_PW"] = sec.MQTT_PW;

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write json to file"));
  }
  file.close();
  //delay(1000);
  unsigned long tn = 0;
  if(millis() > tn + 1000){
    tn = millis();
  }
}


// Prints the content of a file to Serial
void printFile(const char *pfile)
{
  File file = LittleFS.open(pfile, "r");
  if (!file) {
    Serial.println(F("Failed to read file"));
    return;
  }
  // Read each character one by one
  while (file.available()) {
    Serial.print((char)file.read());
  }
  Serial.println();
  file.close();
}


void stopLittleFS()
{
  LittleFS.end();
  //delay(1000);
  unsigned long tn = 0;
  if(millis() > tn + 1000){
    tn = millis();
  }
}


#endif
