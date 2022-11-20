/*########################################################################################
                             Roc-MQTT-Display
Dynamic Passenger Information for Model Railroad Stations controlled by Rocrail or other 
sources via MQTT. A Wemos D1 mini ESP8266 and a TCA9548A I2C Multiplexer can drive up to 
eight I2C OLED displays. Several D1 mini can run together so the total number 
of displays is not limited.

Version 1.09  November 14, 2022

Copyright (c) 2020-2022 Christian Heinrichs. All rights reserved.
https://github.com/chrisweather/RocMQTTdisplay

##########################################################################################

Message Format sent from Rocrail text fields or other sources via MQTT
More details and examples in the Wiki 
https://github.com/chrisweather/RocMQTTdisplay/wiki
######################################################################

  Format: ZZAMSG#Targets#Template#Station#Track#Destination#Departure#Train#TrainType#Message#Spare1#Spare2#

    ZZAMSG:          Identifier for relevant MQTT messages
    Targets:         Identifier for displays, D01 = Display01, D02=Display02, ..., can include more than one Display e.g.: D01D02
    Template:        Identifier for Template, T0 = Template 0, T1 = Template 1, ..., can include only one Template e.g.: T0

    0 - Station:     Name of the station
    1 - Track:       Track number
    2 - Destination: Destination city
    3 - Departure:   Departure time
    4 - Train:       Train number
    5 - TrainType:   Train Type for Logo selection
    6 - Message:     Flexible messages to display either static in the middle of the display or as a ticker at the top.
                     A message in the middle: TPL_xscroll must be 0
                     Example 1: ZZAMSG#D01#T0#Bhf01#1#####Zugdurchfahrt###
                     A message ticker at the top: TPL_xscroll must be 1
    7 - Spare1:      For future features
    8 - Spare2:      For future features

                     Rocrail dynamic text variables can be used https://wiki.rocrail.net/doku.php?id=text-gen-de#dynamischer_text
                     Example 2: ZZAMSG#D01#T0#Bhf01#1#####%lcid%###

                     For dynamic time use:
                     NTP time:     ZZAMSG#D01#T0#Bhf01#1#####{ntptime}###
                     Rocrail time: ZZAMSG#D01#T0#Bhf01#1#####{rrtime}###

                     Clear Display D01 and D02
                     Example 3: ZZAMSG#D01D02###########

    Example 4: ZZAMSG#D01#T0#Hamburg-Hbf#1#Bonn#10:22#ICE 597#ICE####
    Example 5: ZZAMSG#D01#T1#Bhf01#1#Köln-Bonn#10:22#ICE 597#ICE#5min Verspätung###
    Example 6: ZZAMSG#D01D02#T0#Station01#1#Bonn#10:22#IC 56#IC#5min delayed###

    Example 7: Stationname: ZZAMSG#D01#T6#Bogenhausen#########


########################################################################################*/

#include <Arduino.h>
#include <string>
#include <Wire.h>
#include <time.h>
#include "ESP8266WiFi.h"       // https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi
#include <LittleFS.h>          // LittleFS file system https://github.com/esp8266/Arduino/tree/master/libraries/LittleFS
#include "config.h"            // Roc-MQTT-Display configuration file
#include "template.h"          // Roc-MQTT-Display template file
#include "web.h"               // Roc-MQTT-Display web file
#include <ArduinoOTA.h>        // ArduinoOTA by Juraj Andrassy https://github.com/jandrassy/ArduinoOTA
#include "EspMQTTClient.h"     // EspMQTTClient by Patrick Lapointe https://github.com/plapointe6/EspMQTTClient
#include <TaskScheduler.h>     // TaskScheduler by Anatoli Arkhipenko https://github.com/arkhipenko/TaskScheduler
#include <U8g2lib.h>           // U8g2lib by Oliver Kraus https://github.com/olikraus/u8g2
using namespace std;


// !!! SELECT YOUR DISPLAY TYPE HERE !!!
// 
// More U8G2 Display Constructors are listed in the U8G2 Wiki: https://github.com/olikraus/u8g2/wiki/u8g2setupcpp
// Please uncomment ONLY ONE constructor! Only one display type can be handled by one Roc-MQTT-Display controller.

// ### 128x32 ### 0.91" OLED I2C Display with SSD1306 controller, Define OLED Display as disp (D2: SDA, D1: SCL)
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C disp(U8G2_R0, U8X8_PIN_NONE);

// ### 128x64 ### 0.96" OLED I2C Display with SSD1306 controller, Define OLED Display as disp (D2: SDA, D1: SCL)
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C disp(U8G2_R0, U8X8_PIN_NONE);

// ### 64x48 ### 0.66" OLED I2C Display with SSD1306 controller, Define OLED Display as disp (D2: SDA, D1: SCL)
//U8G2_SSD1306_64X48_ER_F_HW_I2C disp(U8G2_R0, U8X8_PIN_NONE);

// ### 96x16 ### 0.69" OLED I2C Display with SSD1306 controller, Define OLED Display as disp (D2: SDA, D1: SCL)
//U8G2_SSD1306_96X16_ER_F_HW_I2C disp(U8G2_R0, U8X8_PIN_NONE);

// These drivers have not been tested with Roc-MQTT-Display
// ### 128x32 ### 0.87" OLED I2C Display with SSD1316 controller, Define OLED Display as disp (D2: SDA, D1: SCL)
//U8G2_SSD1316_128X32_F_HW_I2C disp(U8G2_R0, U8X8_PIN_NONE);


u8g2_uint_t offset1 = 0;  // current offset for the scrolling text
u8g2_uint_t offset2 = 0;
u8g2_uint_t offset3 = 0;
u8g2_uint_t offset4 = 0;
u8g2_uint_t offset5 = 0;
u8g2_uint_t offset6 = 0;
u8g2_uint_t offset7 = 0;
u8g2_uint_t offset8 = 0;
u8g2_uint_t width1 = 0;  // pixel width of the scrolling text (must be < 128 unless U8G2_16BIT is defined, max display 240x240, https://github.com/olikraus/u8g2/wiki/u8g2setupcpp#16-bit-mode)
u8g2_uint_t width2 = 0;
u8g2_uint_t width3 = 0;
u8g2_uint_t width4 = 0;
u8g2_uint_t width5 = 0;
u8g2_uint_t width6 = 0;
u8g2_uint_t width7 = 0;
u8g2_uint_t width8 = 0;

// Define TaskScheduler 
Scheduler ts;

// Callback methods prototypes for TaskScheduler
void send2display1();
void send2display2();
void send2display3();
void send2display4();
void send2display5();
void send2display6();
void send2display7();
void send2display8();

// Tasks for TaskScheduler
Task tS1(5 + config.UPDSPEED, TASK_FOREVER, &send2display1, &ts, true);  // Display 1
Task tS2(5 + config.UPDSPEED, TASK_FOREVER, &send2display2, &ts, false);  // Display 2
Task tS3(5 + config.UPDSPEED, TASK_FOREVER, &send2display3, &ts, false);  // Display 3
Task tS4(5 + config.UPDSPEED, TASK_FOREVER, &send2display4, &ts, false);  // Display 4
Task tS5(5 + config.UPDSPEED, TASK_FOREVER, &send2display5, &ts, false);  // Display 5
Task tS6(5 + config.UPDSPEED, TASK_FOREVER, &send2display6, &ts, false);  // Display 6
Task tS7(5 + config.UPDSPEED, TASK_FOREVER, &send2display7, &ts, false);  // Display 7
Task tS8(5 + config.UPDSPEED, TASK_FOREVER, &send2display8, &ts, false);  // Display 8

// Define WIFI/MQTT Client
EspMQTTClient client( sec.WIFI_SSID, sec.WIFI_PW, config.MQTT_IP, sec.MQTT_USER, sec.MQTT_PW, config.WIFI_DEVICENAME, config.MQTT_PORT );

// Define global variables
unsigned long lastMsg = 0;    // used by ScreenSaver
unsigned long lastNTP = 0;    // used by NTP
time_t now;
tm tm;
String ntptime = "00:00";
String rrtime = "00:00";
String ntpdate = "dd.mm.yyyy";
String rrdate =  "dd.mm.yyyy";

String ZZA1_Targets =     "";
String ZZA1_Template =    "";
String ZZA1_Station =     "";
String ZZA1_Track =       "";
String ZZA1_Destination = "";
String ZZA1_Departure =   "";
String ZZA1_Train =       "";
String ZZA1_Type =        "";
String ZZA1_MessageO =    "";
String ZZA1_Message =     "";
String ZZA1_MessageLoop = "";

String ZZA2_Targets =     "";
String ZZA2_Template =    "";
String ZZA2_Station =     "";
String ZZA2_Track =       "";
String ZZA2_Destination = "";
String ZZA2_Departure =   "";
String ZZA2_Train =       "";
String ZZA2_Type =        "";
String ZZA2_MessageO =    "";
String ZZA2_Message =     "";
String ZZA2_MessageLoop = "";

String ZZA3_Targets =     "";
String ZZA3_Template =    "";
String ZZA3_Station =     "";
String ZZA3_Track =       "";
String ZZA3_Destination = "";
String ZZA3_Departure =   "";
String ZZA3_Train =       "";
String ZZA3_Type =        "";
String ZZA3_MessageO =    "";
String ZZA3_Message =     "";
String ZZA3_MessageLoop = "";

String ZZA4_Targets =     "";
String ZZA4_Template =    "";
String ZZA4_Station =     "";
String ZZA4_Track =       "";
String ZZA4_Destination = "";
String ZZA4_Departure =   "";
String ZZA4_Train =       "";
String ZZA4_Type =        "";
String ZZA4_MessageO =    "";
String ZZA4_Message =     "";
String ZZA4_MessageLoop = "";

String ZZA5_Targets =     "";
String ZZA5_Template =    "";
String ZZA5_Station =     "";
String ZZA5_Track =       "";
String ZZA5_Destination = "";
String ZZA5_Departure =   "";
String ZZA5_Train =       "";
String ZZA5_Type =        "";
String ZZA5_MessageO =    "";
String ZZA5_Message =     "";
String ZZA5_MessageLoop = "";

String ZZA6_Targets =     "";
String ZZA6_Template =    "";
String ZZA6_Station =     "";
String ZZA6_Track =       "";
String ZZA6_Destination = "";
String ZZA6_Departure =   "";
String ZZA6_Train =       "";
String ZZA6_Type =        "";
String ZZA6_MessageO =    "";
String ZZA6_Message =     "";
String ZZA6_MessageLoop = "";

String ZZA7_Targets =     "";
String ZZA7_Template =    "";
String ZZA7_Station =     "";
String ZZA7_Track =       "";
String ZZA7_Destination = "";
String ZZA7_Departure =   "";
String ZZA7_Train =       "";
String ZZA7_Type =        "";
String ZZA7_MessageO =    "";
String ZZA7_Message =     "";
String ZZA7_MessageLoop = "";

String ZZA8_Targets =     "";
String ZZA8_Template =    "";
String ZZA8_Station =     "";
String ZZA8_Track =       "";
String ZZA8_Destination = "";
String ZZA8_Departure =   "";
String ZZA8_Train =       "";
String ZZA8_Type =        "";
String ZZA8_MessageO =    "";
String ZZA8_Message =     "";
String ZZA8_MessageLoop = "";


// SETUP, runs once at startup
void setup()
{
  Serial.begin(115200);
  while (!Serial) continue;
  delay(1000);

  Serial.print(F("\n\n\nStarting Roc-MQTT-Display..."));

  // Initialize LittleFS File System
  if(!LittleFS.begin()){
    Serial.println(F("LittleFS Mount Failed"));
    return;
  }

  TPL = 0;
  // Load sec from file
  Serial.print(F("\nLoading sec from \n"));
  Serial.println(secfile);
  loadSecData(secfile, sec);
  
  // Load config from file
  Serial.print(F("\nLoading configuration from \n"));
  Serial.println(configfile);
  loadConfiguration(configfile, config);

  // Save config to file
  Serial.print(F("\nSaving configuration to \n"));
  Serial.println(configfile);
  saveConfiguration(configfile, config);

  //Read display width and height from display constructor
  config.DISPWIDTH = disp.getDisplayWidth();
  config.DISPHEIGHT = disp.getDisplayHeight();
  
  // Load template data from file
  Serial.print(F("\nLoading template data from \n"));
  Serial.println(templatefile);
  //loadTemplate(templatefile, templ);
  loadTemplate(templatefile);
/*
  Save template data to file
  Serial.print(F("\nSaving template data to "));
  Serial.println(templatefile);
  saveTemplate(templatefile, templ);

  TPL = 0;
  // Save template00 to file
  Serial.print(F("\nSaving template00 to "));
  Serial.println(template00);
  saveTemplateFile(template00);

  TPL = 1;
  // Save template01 to file
  Serial.print(F("\nSaving template01 to "));
  Serial.println(template01);
  saveTemplateFile(template01);

  TPL = 2;
  // Save template02 to file
  Serial.print(F("\nSaving template02 to "));
  Serial.println(template02);
  saveTemplateFile(template02);

  TPL = 3;
  // Save template03 to file
  Serial.print(F("\nSaving template03 to "));
  Serial.println(template03);
  saveTemplateFile(template03);

  TPL = 4;
  // Save template04 to file
  Serial.print(F("\nSaving template04 to "));
  Serial.println(template04);
  saveTemplateFile(template04);

  TPL = 5;
  // Save template05 to file
  Serial.print(F("\nSaving template05 to "));
  Serial.println(template05);
  saveTemplateFile(template05);

  TPL = 6;
  // Save template06 to file
  Serial.print(F("\nSaving template06 to "));
  Serial.println(template06);
  saveTemplateFile(template06);

  TPL = 7;
  // Save template07 to file
  Serial.print(F("\nSaving template07 to "));
  Serial.println(template07);
  saveTemplateFile(template07);

  TPL = 8;
  // Save template08 to file
  Serial.print(F("\nSaving template08 to "));
  Serial.println(template08);
  saveTemplateFile(template08);

  TPL = 9;
  // Save template09 to file
  Serial.print(F("\nSaving template09 to "));
  Serial.println(template09);
  saveTemplateFile(template09);
*/
  // Load template0x from file
  Serial.print(F("\nLoading templates from \n"));

  TPL = 0;
  Serial.println(template00);
  loadTemplateFile(template00);

  TPL = 1;
  Serial.println(template01);
  loadTemplateFile(template01);

  TPL = 2;
  Serial.println(template02);
  loadTemplateFile(template02);

  TPL = 3;
  Serial.println(template03);
  loadTemplateFile(template03);

  TPL = 4;  
  Serial.println(template04);
  loadTemplateFile(template04);

  TPL = 5;
  Serial.println(template05);
  loadTemplateFile(template05);

  TPL = 6;
  Serial.println(template06);
  loadTemplateFile(template06);

  TPL = 7;
  Serial.println(template07);
  loadTemplateFile(template07);

  TPL = 8;
  Serial.println(template08);
  loadTemplateFile(template08);

  TPL = 9;
  Serial.println(template09);
  loadTemplateFile(template09);
  TPL = 0;

  if (config.MQTT_DEBUG == 1){
    // Dump configuration files
    Serial.print(F("\nPrint "));
    Serial.println(secfile);
    printFile(secfile);
  
    Serial.print(F("\nPrint "));
    Serial.println(configfile);
    printFile(configfile);

    Serial.print(F("\nPrint "));
    Serial.println(templatefile);
    printFile(templatefile);
  
    Serial.print(F("\nPrint "));
    Serial.println(template00);
    printFile(template00);

    Serial.print(F("\nPrint "));
    Serial.println(template01);
    printFile(template01);

    Serial.print(F("\nPrint "));
    Serial.println(template02);
    printFile(template02);

    Serial.print(F("\nPrint "));
    Serial.println(template03);
    printFile(template03);

    Serial.print(F("\nPrint "));
    Serial.println(template04);
    printFile(template04);

    Serial.print(F("\nPrint "));
    Serial.println(template05);
    printFile(template05);

    Serial.print(F("\nPrint "));
    Serial.println(template06);
    printFile(template06);

    Serial.print(F("\nPrint "));
    Serial.println(template07);
    printFile(template07);

    Serial.print(F("\nPrint "));
    Serial.println(template08);
    printFile(template08);

    Serial.print(F("\nPrint "));
    Serial.println(template09);
    printFile(template09);
  }

  // Switch off Wemos D1 mini onboard LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);

  Serial.println(F("\nRoc-MQTT-Display"));
  Serial.println(config.VER);
  Serial.println(F("FOR DEBUG INFORMATION set 'Enable debug messages' to 1 in CONFIGURATION"));
  //Serial.print(F("\nCurrent MQTT broker IP-adress: "));
  //Serial.println(config.MQTT_IP);
  if (strlen(config.MQTT_IP) < 7) {
    Serial.println(F("\nWARNING: MQTT broker IP-adress is missing or incomplete in CONFIGURATION"));
  }
  //Serial.print(F("\nFOR CONFIGURATION OPEN: http://"));
  //Serial.println(config.WIFI_DEVICENAME);
  Serial.print(F("\n  Displays enabled: "));
  Serial.print(config.NUMDISP);
  Serial.println(F(" / 8"));
  Serial.print(F("  Display size: "));
  Serial.print(config.DISPWIDTH);
  Serial.print(F(" x "));
  Serial.println(config.DISPHEIGHT);

  // Initialize all connected displays
  if (config.MUX == 0){
    tS1.setInterval(10 + config.UPDSPEED);
    tS2.disable();
    tS3.disable();
    tS4.disable();
    tS5.disable();
    tS6.disable();
    tS7.disable();
    tS8.disable();
  }

  if(config.MUX > 0){
    Wire.begin();
  }
  DisplayInit();

  // Optional functionalities of EspMQTTClient
  client.enableDebuggingMessages(config.MQTT_DEBUG);
  client.setMaxPacketSize(config.MQTT_MSGSIZE);
  client.setKeepAlive(config.MQTT_KEEPALIVE1);
  client.setMqttReconnectionAttemptDelay(config.MQTT_RECONDELAY);
  client.setWifiReconnectionAttemptDelay(config.WIFI_RECONDELAY);

  // Initialize NTP time client
  configTime(config.NTP_TZ, config.NTP_SERVER);

  // OTA
  ArduinoOTA.setPort(config.OTA_PORT);
  ArduinoOTA.setHostname(config.OTA_HOSTNAME);
  ArduinoOTA.setPassword(sec.OTA_PW);
  ArduinoOTA.setPasswordHash(sec.OTA_HASH);

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }
    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("OTA Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println(F("\nOTA End"));
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println(F("OTA Authentication Failed"));
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println(F("OTA Begin Failed"));
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println(F("OTA Connect Failed"));
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println(F("OTA Receive Failed"));
    } else if (error == OTA_END_ERROR) {
      Serial.println(F("OTA End Failed"));
    }
  });
  ArduinoOTA.begin();
  //Serial.print(F("\nOTA Ready -> Port: "));
  //Serial.print(config.OTA_HOSTNAME);
  //Serial.print(F(" at "));
  //Serial.println(WiFi.localIP());

  // Initialize WEBSERVER
  webserver.on("/", []() {             // Define the handling function for / path
    loadRoot();
  });

  webserver.on("/index.htm", []() {    // Define the handling function for /index.htm path
    loadRoot();
  });

  webserver.on("/favicon.ico", []() {  // Define the handling function for favicon requests
    webserver.send(204);
  });

  webserver.on("/rmd.css", []() {      // Define the handling function for css file requests
    loadCSS();
  });

  webserver.on("/config", []() {       // Define the handling function for the /config path
    loadCfg();
  });

  webserver.on("/tpl1", []() {         // Define the handling function for the /tpl1 path
    loadTpl1();
  });

  webserver.on("/tpl1sel", []() {      // Define the handling function for the /tpl1sel path
    handleTpl1Select();
  });

  webserver.on("/tpl2", []() {         // Define the handling function for the /tpl2 path
    loadTpl2();
  });

  webserver.on("/tpl2sel", []() {      // Define the handling function for the /tpl2sel path
    handleTpl2Select();
  });

webserver.on("/tpl2imp", []() {         // Define the handling function for the /tpl2imp path
    loadTpl2imp();
  });

  webserver.on("/sec", []() {          // Define the handling function for the /sec path
    loadSec();
  });

  webserver.on("/submitcfg", []() {    // Define the handling function for the /submitcfg path
    webserver.send(204);
    handleCfgSubmit();
    loadCfg();
  });

  webserver.on("/submittpl1", []() {   // Define the handling function for the /submittpl1 path
    webserver.send(204);
    handleTpl1Submit();
    loadTpl1();
  });

  webserver.on("/submittpl2", []() {   // Define the handling function for the /submittpl2 path
    webserver.send(204);
    handleTpl2Submit();
    loadTpl2();
  });

  webserver.on("/submittpl2imp", []() {   // Define the handling function for the /submittpl2imp path
    webserver.send(204);
    handleTpl2impSubmit();
  });

  webserver.on("/submitsec", []() {    // Define the handling function for the /submitsec path
    webserver.send(204);
    handleSecSubmit();
    loadSec();
  });

  webserver.on("/download", []() {     // Define the handling function for the /download path
    //webserver.send(204);
    downloadFile();
  });

  webserver.on("/printdisp", []() {    // Define the handling function for the /printdisp path
    webserver.send(204);
    //printBuffer();
    config.PRINTBUF = 1;
  });

  webserver.on("/restart", []() {      // Define the handling function for the /restart path
    webserver.send(204);
    unsigned long tn = 0;
    if(millis() > tn + 300){
      tn = millis();
    }
    restartESP();
  });

  webserver.on("/ota", []() {          // Define the handling function for the /ota path
    webserver.send(204);
    unsigned long tn = 0;
    if(millis() > tn + 500){
      tn = millis();
    }
    stopLittleFS();
  });

  webserver.onNotFound([]() {          // Define the handling function for Site Not Found response
    loadNotFound();
  });

  webserver.begin();                   // Start the webserver
  Serial.println(F("\nWebserver started and listening for requests"));

} // End of setup


// Restart the controller
void restartESP()
{
  stopLittleFS();
  ESP.restart();
}


// Display Switch with I2C Display Multiplexer TCA9548A
void DMUX(uint8_t port)
{
  Wire.beginTransmission( config.MUX );  // TCA9548A default address is 0x70
  Wire.write( 1 << port );               // Send byte to select display port
  Wire.endTransmission();
}


// Initialize all connected displays
void DisplayInit()
{
  // Loop through all connected displays on the I2C bus
  for (uint8_t i = 0; i < config.NUMDISP; i++) {
    switch (i){
    case 0: tS1.enable();
            //tS1.setInterval(10 + config.UPDSPEED);
            break;
    case 1: tS2.enable();
            break;
    case 2: tS3.enable();
            break;
    case 3: tS4.enable();
            break;
    case 4: tS5.enable();
            break;
    case 5: tS6.enable();
            break;
    case 6: tS7.enable();
            break;
    case 7: tS8.enable();
            break;
    }
    if (config.MUX > 0){
      DMUX(i);
    }
    disp.clearBuffer();
    disp.firstPage();
    do {
      disp.begin();  // Initialize display i
      disp.setFlipMode(DPL_flip[i]);
      //disp.setDisplayRotation(U8G2_R2);  // U8G2_R0, U8G2_R1, U8G2_R2, U8G2_R3, U8G2_MIRROR  DPL_rotation[i]
      disp.setContrast(DPL_contrast[i]);
      //if (DPL_contrast[i] == 0){
      //  disp.setPowerSave(1);
      //}
      //else {
      //  disp.setPowerSave(0);
      //}
      disp.enableUTF8Print();
      disp.setFont(fontno[5]);
      disp.setFontMode(0);
      disp.setCursor(0,7);
      disp.print(F("Roc-MQTT-Display "));
      disp.setCursor(0,15);
      disp.print(config.VER);
      disp.nextPage();
      delay(500 + (config.STARTDELAY / 2));
      disp.clearDisplay();
      disp.setFont(fontno[5]);
      disp.setCursor(0,7);
      disp.print(F("http://"));
      disp.setCursor(0,15);
      disp.print(config.WIFI_DEVICENAME);
      disp.nextPage();
      delay(500 + (config.STARTDELAY / 2));
      disp.clearDisplay();
      disp.setCursor(0,7);
      disp.print(F("Display: "));
      disp.print(i+1);
      disp.setCursor(0,15);
      disp.print(F("ID: "));
      disp.print(DPL_id[i]);
      
      if (strlen(config.MQTT_IP) < 7) {
        disp.nextPage();
        delay(500 + (config.STARTDELAY / 2));
        disp.clearDisplay();
        disp.setCursor(0,7);
        disp.print(F("NO MQTT broker"));
        disp.setCursor(0,15);
        disp.print(F("Check Config!"));
        disp.nextPage();
        delay(5000 + (config.STARTDELAY / 2));
      }
      Serial.print(F("  Display: "));
      Serial.print(i+1);
      Serial.print(F("  connected to multiplexer port (SCx, SDx): "));
      Serial.print(i);
      Serial.print(F("  Display-ID: "));
      Serial.println(DPL_id[i]);
    } while (disp.nextPage());
  }
  delay(config.STARTDELAY);
}


// TaskScheduler callback methods 1-8

// *** Write to Display 1 ***
void send2display1(void)
{ 
  // Template number
  uint8_t t = ZZA1_Template.toInt();
  if (t > 9){
    t = 0;
  }
  if (DPL_side[0] == 1){
    if (TPL_side[t] == 0 && TPL_side[t+1] == 1){
      t = t+1;
    }
  }
  if (config.MUX > 0){
    DMUX(0);
  }
  u8g2_uint_t x;
  disp.firstPage();
  if (TPL_invert[t] == 1){
    disp.sendF("c", 0x0a7);
  }
  else {
    disp.sendF("c", 0x0a6);
  }
  //disp.setContrast(DPL_contrast[0]);
  // Message
  // *** Message only, in the middle of the display ***
  if (TPL_6scroll[t] != 1){
    if (ZZA1_Message.length() > 1){
      disp.setFont(fontno[TPL_6font[t]]);
      // Switch to narrow font for longer messages
      if (disp.getUTF8Width(ZZA1_Message.c_str()) > TPL_6maxwidth[t]){
        disp.setFont(fontno[TPL_6font2[t]]);
      }
      disp.setFontMode(TPL_6fontmode[t]);
      disp.setDrawColor(TPL_6drawcolor[t]);
      
      disp.setCursor(TPL_6posx[t] + (TPL_6maxwidth[t] / 2) - (disp.getUTF8Width(ZZA1_Message.c_str()) / 2), TPL_6posy[t]);
      disp.print(ZZA1_Message);
    }
  }
  else {
    // *** Scrolling message on top ***
    if (ZZA1_Message.length() > 1){
      disp.setFont(fontno[TPL_6font[t]]);
      width1 = disp.getUTF8Width(ZZA1_MessageLoop.c_str());
      disp.setFontMode(TPL_6fontmode[t]);
      disp.setDrawColor(TPL_6drawcolor[t]);
      if (TPL_6boxh[t] > 0){
        // draw message box
        disp.drawBox(TPL_6boxx[t], TPL_6boxy[t], TPL_6boxw[t], TPL_6boxh[t]);
        // draw black box
        disp.setFontMode(TPL_6fontmode2[t]);
        disp.setDrawColor(TPL_6drawcolor2[t]);
        disp.drawBox(TPL_6box2x[t], TPL_6box2y[t], TPL_6box2w[t], TPL_6box2h[t]);
      }
      x = offset1;
      do {
        disp.drawUTF8(x, TPL_6posy[t], ZZA1_MessageLoop.c_str());  // !!!!!!!!!!!!!
        x += width1;
      //} while( x < disp.getDisplayWidth());
      } while( x < config.DISPWIDTH);
    }
  }
  disp.setFontMode(TPL_6fontmode[t]);
  disp.setDrawColor(TPL_6drawcolor[t]);

  // Station
  disp.setFont(fontno[TPL_0font[t]]);
  //Serial.println(ZZA1_Station);
  //if (int a = ZZA1_Station.indexOf("\n") != -1){
    //Serial.println("line break");
    //Serial.println(a);
  //}
  //int start09 = start08 + 1 + pld.substring(start08).indexOf("#");
  
  // Switch to narrow font for longer station names
  if (disp.getUTF8Width(ZZA1_Station.c_str()) > TPL_0maxwidth[t]){
    disp.setFont(fontno[TPL_0font2[t]]);
  }
  //disp.setCursor((disp.getDisplayWidth() / 2) - (disp.getUTF8Width(ZZA2_Station.c_str()) / 2), TPL_3posy[t]);
  //disp.drawUTF8(TPL_0posx[t], TPL_0posy[t], ZZA2_Station.c_str());
  disp.drawUTF8((config.DISPWIDTH / 2) - (disp.getUTF8Width(ZZA1_Station.c_str()) / 2) + TPL_0posx[t], TPL_0posy[t], ZZA1_Station.c_str());

  // Track
  disp.setFont(fontno[TPL_1font[t]]);
  disp.setCursor(TPL_1posx[t],TPL_1posy[t]);
  disp.print(ZZA1_Track);

  // Destination
  disp.setFont(fontno[TPL_2font[t]]);
  // Switch to narrow font for longer destination names
  if (disp.getUTF8Width(ZZA1_Destination.c_str()) > TPL_2maxwidth[t]){
    disp.setFont(fontno[TPL_2font2[t]]);
  }
  disp.drawUTF8(TPL_2posx[t], TPL_2posy[t], ZZA1_Destination.c_str());

  // Departure
  disp.setFont(fontno[TPL_3font[t]]);
  //disp.setCursor(disp.getDisplayWidth() - disp.getUTF8Width(ZZA1_Departure.c_str()), TPL_3posy[t]);
  disp.setCursor(TPL_3posx[t], TPL_3posy[t]);
  disp.print(ZZA1_Departure);

  // Train
  disp.setFont(fontno[TPL_4font[t]]);
  //disp.setCursor(disp.getDisplayWidth() - disp.getUTF8Width(ZZA1_Train.c_str()), TPL_4posy[t]);
  disp.setCursor(TPL_4posx[t], TPL_4posy[t]);
  disp.print(ZZA1_Train);

  // Logo
  if ( ZZA1_Type != "" ) {
    switchLogo(t, ZZA1_Type);
  }

/*  int l = ZZA1_Logo;
    
    switch (l){ 
      //case 1: disp.drawXBM( TPL_6logox[t], TPL_6logoy[t], TPL_6logow[t], TPL_6logoh[t], switchLogo(ZZA1_Logo));
      case 1: disp.drawXBM( TPL_6logox[t], TPL_6logoy[t], TPL_6logow[t], TPL_6logoh[t], *pl1);
              break;
      case 2: disp.drawXBM( TPL_6logox[t], TPL_6logoy[t], TPL_6logow[t], TPL_6logoh[t], *pl2);
              break;
      case 3: disp.drawXBM( TPL_6logox[t], TPL_6logoy[t], TPL_6logow[t], TPL_6logoh[t], logo3);
              break;
      case 4: disp.drawXBM( TPL_6logox[t], TPL_6logoy[t], TPL_6logow[t], TPL_6logoh[t], *pl4);
              break;
      case 5: disp.drawXBM( TPL_6logox[t], TPL_6logoy[t], TPL_6logow[t], TPL_6logoh[t], *pl5);
              break;
      case 6: disp.drawXBM( TPL_6logox[t], TPL_6logoy[t], TPL_6logow[t], TPL_6logoh[t], logo6);
              break;
      case 7: disp.drawXBM( TPL_6logox[t], TPL_6logoy[t], TPL_6logow[t], TPL_6logoh[t], *pl7);
              break;
      case 8: disp.drawXBM( TPL_6logox[t], TPL_6logoy[t], TPL_6logow[t], TPL_6logoh[t], *pl8);
              break;
      case 9: disp.drawXBM( TPL_6logox[t], TPL_6logoy[t], TPL_6logow[t], TPL_6logoh[t], logo9);
    }
  }
*/
  
  disp.nextPage();
  if (config.PRINTBUF == 1){
    printBuffer();
    config.PRINTBUF = 0;
  }
  offset1-=1;
  if ( (u8g2_uint_t)offset1 < (u8g2_uint_t)-width1 )
    offset1 = 0;
}


// *** Write to Display 2 ***
void send2display2(void)
{ 
  // Template number
  uint8_t t = ZZA2_Template.toInt();
  if (t > 9){
    t = 0;
  }
  if (DPL_side[1] == 1){
    if (TPL_side[t] == 0 && TPL_side[t+1] == 1){
      t = t+1;
    }
  }

  DMUX(1);
  u8g2_uint_t x;
  disp.firstPage();
  if (TPL_invert[t] == 1){
    disp.sendF("c", 0x0a7);
  }
  else {
    disp.sendF("c", 0x0a6);
  }
  //disp.setContrast(DPL_contrast[1]);
  // Message
  // *** Message only, in the middle of the display ***
  if (TPL_6scroll[t] != 1){
    if (ZZA2_Message.length() > 1){
      disp.setFont(fontno[TPL_6font[t]]);
      // Switch to narrow font for longer messages
      if (disp.getUTF8Width(ZZA2_Message.c_str()) > TPL_6maxwidth[t]){
        disp.setFont(fontno[TPL_6font2[t]]);
      }
      disp.setFontMode(TPL_6fontmode[t]);
      disp.setDrawColor(TPL_6drawcolor[t]);
      
      disp.setCursor(TPL_6posx[t] + (TPL_6maxwidth[t] / 2) - (disp.getUTF8Width(ZZA2_Message.c_str()) / 2), TPL_6posy[t]);
      disp.print(ZZA2_Message);
    }
  }
  else {
    // *** Scrolling message on top ***
    if (ZZA2_Message.length() > 1){
      disp.setFont(fontno[TPL_6font[t]]);
      width2 = disp.getUTF8Width(ZZA2_MessageLoop.c_str());
      disp.setFontMode(TPL_6fontmode[t]);
      disp.setDrawColor(TPL_6drawcolor[t]);
      if (TPL_6boxh[t] > 0){
        disp.drawBox(TPL_6boxx[t], TPL_6boxy[t], TPL_6boxw[t], TPL_6boxh[t]);
        disp.setFontMode(TPL_6fontmode2[t]);
        disp.setDrawColor(TPL_6drawcolor2[t]);
        disp.drawBox(TPL_6box2x[t], TPL_6box2y[t], TPL_6box2w[t], TPL_6box2h[t]);
      }
      x = offset2;
      do {
        disp.drawUTF8(x, 8, ZZA2_MessageLoop.c_str());
        x += width2;
      } while( x < config.DISPWIDTH);
    }
  }
  disp.setFontMode(TPL_6fontmode[t]);
  disp.setDrawColor(TPL_6drawcolor[t]);

  // Station
  disp.setFont(fontno[TPL_0font[t]]);
  // Switch to narrow font for longer station names
  if (disp.getUTF8Width(ZZA2_Station.c_str()) > TPL_0maxwidth[t]){
    disp.setFont(fontno[TPL_0font2[t]]);
  }
  disp.drawUTF8((config.DISPWIDTH / 2) - (disp.getUTF8Width(ZZA2_Station.c_str()) / 2) + TPL_0posx[t], TPL_0posy[t], ZZA2_Station.c_str());

  // Track
  disp.setFont(fontno[TPL_1font[t]]);
  disp.setCursor(TPL_1posx[t],TPL_1posy[t]);
  disp.print(ZZA2_Track);

  // Destination
  disp.setFont(fontno[TPL_2font[t]]);
  // Switch to narrow font for longer destination names
  if (disp.getUTF8Width(ZZA2_Destination.c_str()) > TPL_2maxwidth[t]){
    disp.setFont(fontno[TPL_2font2[t]]);
  }
  disp.drawUTF8(TPL_2posx[t], TPL_2posy[t], ZZA2_Destination.c_str());

  // Departure
  disp.setFont(fontno[TPL_3font[t]]);
  //disp.setCursor(disp.getDisplayWidth() - disp.getUTF8Width(ZZA2_Departure.c_str()), TPL_3posy[t]);
  disp.setCursor(TPL_3posx[t], TPL_3posy[t]);
  disp.print(ZZA2_Departure);

  // Train
  disp.setFont(fontno[TPL_4font[t]]);
  //disp.setCursor(disp.getDisplayWidth() - disp.getUTF8Width(ZZA2_Train.c_str()), TPL_4posy[t]);
  disp.setCursor(TPL_4posx[t], TPL_4posy[t]);
  disp.print(ZZA2_Train);

  // Logo
  if ( ZZA2_Type != "" ) {
    switchLogo(t, ZZA2_Type);
  }

  disp.nextPage();

  offset2-=1;
  if ( (u8g2_uint_t)offset2 < (u8g2_uint_t)-width2 )
    offset2 = 0;
}


// *** Write to Display 3 ***
void send2display3(void)
{ 
  // Template number
  uint8_t t = ZZA3_Template.toInt();
  if (t > 9){
    t = 0;
  }
  if (DPL_side[2] == 1){
    if (TPL_side[t] == 0 && TPL_side[t+1] == 1){
      t = t+1;
    }
  }

  DMUX(2);
  u8g2_uint_t x;
  disp.firstPage();
  if (TPL_invert[t] == 1){
    disp.sendF("c", 0x0a7);
  }
  else {
    disp.sendF("c", 0x0a6);
  }
  //disp.setContrast(DPL_contrast[2]);
  // Message
  // *** Message only, in the middle of the display ***
  if (TPL_6scroll[t] != 1){
    if (ZZA3_Message.length() > 1){
      disp.setFont(fontno[TPL_6font[t]]);
      // Switch to narrow font for longer messages
      if (disp.getUTF8Width(ZZA3_Message.c_str()) > TPL_6maxwidth[t]){
        disp.setFont(fontno[TPL_6font2[t]]);
      }
      disp.setFontMode(TPL_6fontmode[t]);
      disp.setDrawColor(TPL_6drawcolor[t]);
      
      disp.setCursor(TPL_6posx[t] + (TPL_6maxwidth[t] / 2) - (disp.getUTF8Width(ZZA3_Message.c_str()) / 2), TPL_6posy[t]);
      disp.print(ZZA3_Message);
    }
  }
  else {
    // *** Scrolling message on top ***
    if (ZZA3_Message.length() > 1){
      disp.setFont(fontno[TPL_6font[t]]);
      width3 = disp.getUTF8Width(ZZA3_MessageLoop.c_str());
      disp.setFontMode(TPL_6fontmode[t]);
      disp.setDrawColor(TPL_6drawcolor[t]);
      if (TPL_6boxh[t] > 0){
        disp.drawBox(TPL_6boxx[t], TPL_6boxy[t], TPL_6boxw[t], TPL_6boxh[t]);
        disp.setFontMode(TPL_6fontmode2[t]);
        disp.setDrawColor(TPL_6drawcolor2[t]);
        disp.drawBox(TPL_6box2x[t], TPL_6box2y[t], TPL_6box2w[t], TPL_6box2h[t]);
      }
      x = offset3;
      do {
        disp.drawUTF8(x, 8, ZZA3_MessageLoop.c_str());
        x += width3;
      } while( x < config.DISPWIDTH);
    }
  }
  disp.setFontMode(TPL_6fontmode[t]);
  disp.setDrawColor(TPL_6drawcolor[t]);

  // Station
  disp.setFont(fontno[TPL_0font[t]]);
  // Switch to narrow font for longer station names
  if (disp.getUTF8Width(ZZA3_Station.c_str()) > TPL_0maxwidth[t]){
    disp.setFont(fontno[TPL_0font2[t]]);
  }
  disp.drawUTF8((config.DISPWIDTH / 2) - (disp.getUTF8Width(ZZA3_Station.c_str()) / 2) + TPL_0posx[t], TPL_0posy[t], ZZA3_Station.c_str());

  // Track
  disp.setFont(fontno[TPL_1font[t]]);
  disp.setCursor(TPL_1posx[t],TPL_1posy[t]);
  disp.print(ZZA3_Track);

  // Destination
  disp.setFont(fontno[TPL_2font[t]]);
  // Switch to narrow font for longer destination names
  if (disp.getUTF8Width(ZZA3_Destination.c_str()) > TPL_2maxwidth[t]){
    disp.setFont(fontno[TPL_2font2[t]]);
  }
  disp.drawUTF8(TPL_2posx[t], TPL_2posy[t], ZZA3_Destination.c_str());

  // Departure
  disp.setFont(fontno[TPL_3font[t]]);
  //disp.setCursor(disp.getDisplayWidth() - disp.getUTF8Width(ZZA3_Departure.c_str()), TPL_3posy[t]);
  disp.setCursor(TPL_3posx[t], TPL_3posy[t]);
  disp.print(ZZA3_Departure);

  // Train
  disp.setFont(fontno[TPL_4font[t]]);
  //disp.setCursor(disp.getDisplayWidth() - disp.getUTF8Width(ZZA3_Train.c_str()), TPL_4posy[t]);
  disp.setCursor(TPL_4posx[t], TPL_4posy[t]);
  disp.print(ZZA3_Train);

  // Logo
  if ( ZZA3_Type != "" ) {
    switchLogo(t, ZZA3_Type);
  }
  
  disp.nextPage();

  offset3-=1;
  if ( (u8g2_uint_t)offset3 < (u8g2_uint_t)-width3 )
    offset3 = 0;
}


// *** Write to Display 4 ***
void send2display4(void)
{ 
  // Template number
  uint8_t t = ZZA4_Template.toInt();
  if (t > 9){
    t = 0;
  }
  if (DPL_side[3] == 1){
    if (TPL_side[t] == 0 && TPL_side[t+1] == 1){
      t = t+1;
    }
  }

  DMUX(3);
  u8g2_uint_t x;
  disp.firstPage();
  if (TPL_invert[t] == 1){
    disp.sendF("c", 0x0a7);
  }
  else {
    disp.sendF("c", 0x0a6);
  }
  //disp.setContrast(DPL_contrast[3]);
  // Message
  // *** Message only, in the middle of the display ***
  if (TPL_6scroll[t] != 1){
    if (ZZA4_Message.length() > 1){
      disp.setFont(fontno[TPL_6font[t]]);
      // Switch to narrow font for longer messages
      if (disp.getUTF8Width(ZZA4_Message.c_str()) > TPL_6maxwidth[t]){
        disp.setFont(fontno[TPL_6font2[t]]);
      }
      disp.setFontMode(TPL_6fontmode[t]);
      disp.setDrawColor(TPL_6drawcolor[t]);
      
      disp.setCursor(TPL_6posx[t] + (TPL_6maxwidth[t] / 2) - (disp.getUTF8Width(ZZA4_Message.c_str()) / 2), TPL_6posy[t]);
      disp.print(ZZA4_Message);
    }
  }
  else {
    // *** Scrolling message on top ***
    if (ZZA4_Message.length() > 1){
      disp.setFont(fontno[TPL_6font[t]]);
      width4 = disp.getUTF8Width(ZZA4_MessageLoop.c_str());
      disp.setFontMode(TPL_6fontmode[t]);
      disp.setDrawColor(TPL_6drawcolor[t]);
      if (TPL_6boxh[t] > 0){
        disp.drawBox(TPL_6boxx[t], TPL_6boxy[t], TPL_6boxw[t], TPL_6boxh[t]);
        disp.setFontMode(TPL_6fontmode2[t]);
        disp.setDrawColor(TPL_6drawcolor2[t]);
        disp.drawBox(TPL_6box2x[t], TPL_6box2y[t], TPL_6box2w[t], TPL_6box2h[t]);
      }
      x = offset4;
      do {
        disp.drawUTF8(x, 8, ZZA4_MessageLoop.c_str());
        x += width4;
      } while( x < disp.getDisplayWidth());
    }
  }
  disp.setFontMode(TPL_6fontmode[t]);
  disp.setDrawColor(TPL_6drawcolor[t]);

  // Station
  disp.setFont(fontno[TPL_0font[t]]);
  // Switch to narrow font for longer station names
  if (disp.getUTF8Width(ZZA4_Station.c_str()) > TPL_0maxwidth[t]){
    disp.setFont(fontno[TPL_0font2[t]]);
  }
  disp.drawUTF8((config.DISPWIDTH / 2) - (disp.getUTF8Width(ZZA4_Station.c_str()) / 2) + TPL_0posx[t], TPL_0posy[t], ZZA4_Station.c_str());

  // Track
  disp.setFont(fontno[TPL_1font[t]]);
  disp.setCursor(TPL_1posx[t],TPL_1posy[t]);
  disp.print(ZZA4_Track);

  // Destination
  disp.setFont(fontno[TPL_2font[t]]);
  // Switch to narrow font for longer destination names
  if (disp.getUTF8Width(ZZA4_Destination.c_str()) > TPL_2maxwidth[t]){
    disp.setFont(fontno[TPL_2font2[t]]);
  }
  disp.drawUTF8(TPL_2posx[t], TPL_2posy[t], ZZA4_Destination.c_str());

  // Departure
  disp.setFont(fontno[TPL_3font[t]]);
  //disp.setCursor(disp.getDisplayWidth() - disp.getUTF8Width(ZZA4_Departure.c_str()), TPL_3posy[t]);
  disp.setCursor(TPL_3posx[t], TPL_3posy[t]);
  disp.print(ZZA4_Departure);

  // Train
  disp.setFont(fontno[TPL_4font[t]]);
  //disp.setCursor(disp.getDisplayWidth() - disp.getUTF8Width(ZZA4_Train.c_str()), TPL_4posy[t]);
  disp.setCursor(TPL_4posx[t], TPL_4posy[t]);
  disp.print(ZZA4_Train);

  // Logo
  if ( ZZA4_Type != "" ) {
    switchLogo(t, ZZA4_Type);
  }
  
  disp.nextPage();

  offset4-=1;
  if ( (u8g2_uint_t)offset4 < (u8g2_uint_t)-width4 )
    offset4 = 0;
}


// *** Write to Display 5 ***
void send2display5(void)
{ 
  // Template number
  uint8_t t = ZZA5_Template.toInt();
  if (t > 9){
    t = 0;
  }
  if (DPL_side[4] == 1){
    if (TPL_side[t] == 0 && TPL_side[t+1] == 1){
      t = t+1;
    }
  }

  DMUX(4);
  u8g2_uint_t x;
  disp.firstPage();
  if (TPL_invert[t] == 1){
    disp.sendF("c", 0x0a7);
  }
  else {
    disp.sendF("c", 0x0a6);
  }
  //disp.setContrast(DPL_contrast[4]);
  // Message
  // *** Message only, in the middle of the display ***
  if (TPL_6scroll[t] != 1){
    if (ZZA5_Message.length() > 1){
      disp.setFont(fontno[TPL_6font[t]]);
      // Switch to narrow font for longer messages
      if (disp.getUTF8Width(ZZA5_Message.c_str()) > TPL_6maxwidth[t]){
        disp.setFont(fontno[TPL_6font2[t]]);
      }
      disp.setFontMode(TPL_6fontmode[t]);
      disp.setDrawColor(TPL_6drawcolor[t]);
      
      disp.setCursor(TPL_6posx[t] + (TPL_6maxwidth[t] / 2) - (disp.getUTF8Width(ZZA5_Message.c_str()) / 2), TPL_6posy[t]);
      disp.print(ZZA5_Message);
    }
  }
  else {
    // *** Scrolling message on top ***
    if (ZZA5_Message.length() > 1){
      disp.setFont(fontno[TPL_6font[t]]);
      width5 = disp.getUTF8Width(ZZA5_MessageLoop.c_str());
      disp.setFontMode(TPL_6fontmode[t]);
      disp.setDrawColor(TPL_6drawcolor[t]);
      if (TPL_6boxh[t] > 0){
        disp.drawBox(TPL_6boxx[t], TPL_6boxy[t], TPL_6boxw[t], TPL_6boxh[t]);
        disp.setFontMode(TPL_6fontmode2[t]);
        disp.setDrawColor(TPL_6drawcolor2[t]);
        disp.drawBox(TPL_6box2x[t], TPL_6box2y[t], TPL_6box2w[t], TPL_6box2h[t]);
      }
      x = offset5;
      do {
        disp.drawUTF8(x, 8, ZZA5_MessageLoop.c_str());
        x += width5;
      } while( x < config.DISPWIDTH);
    }
  }
  disp.setFontMode(TPL_6fontmode[t]);
  disp.setDrawColor(TPL_6drawcolor[t]);

  // Station
  disp.setFont(fontno[TPL_0font[t]]);
  // Switch to narrow font for longer station names
  if (disp.getUTF8Width(ZZA5_Station.c_str()) > TPL_0maxwidth[t]){
    disp.setFont(fontno[TPL_0font2[t]]);
  }
  disp.drawUTF8((config.DISPWIDTH / 2) - (disp.getUTF8Width(ZZA5_Station.c_str()) / 2) + TPL_0posx[t], TPL_0posy[t], ZZA5_Station.c_str());

  // Track
  disp.setFont(fontno[TPL_1font[t]]);
  disp.setCursor(TPL_1posx[t],TPL_1posy[t]);
  disp.print(ZZA5_Track);

  // Destination
  disp.setFont(fontno[TPL_2font[t]]);
  // Switch to narrow font for longer destination names
  if (disp.getUTF8Width(ZZA5_Destination.c_str()) > TPL_2maxwidth[t]){
    disp.setFont(fontno[TPL_2font2[t]]);
  }
  disp.drawUTF8(TPL_2posx[t], TPL_2posy[t], ZZA5_Destination.c_str());

  // Departure
  disp.setFont(fontno[TPL_3font[t]]);
  //disp.setCursor(disp.getDisplayWidth() - disp.getUTF8Width(ZZA5_Departure.c_str()), TPL_3posy[t]);
  disp.setCursor(TPL_3posx[t], TPL_3posy[t]);
  disp.print(ZZA5_Departure);

  // Train
  disp.setFont(fontno[TPL_4font[t]]);
  //disp.setCursor(disp.getDisplayWidth() - disp.getUTF8Width(ZZA5_Train.c_str()), TPL_4posy[t]);
  disp.setCursor(TPL_4posx[t], TPL_4posy[t]);
  disp.print(ZZA5_Train);

  // Logo
  if ( ZZA5_Type != "" ) {
    switchLogo(t, ZZA5_Type);
  }
  
  disp.nextPage();

  offset5-=1;
  if ( (u8g2_uint_t)offset5 < (u8g2_uint_t)-width5 )
    offset5 = 0;
}


// *** Write to Display 6 ***
void send2display6(void)
{ 
  // Template number
  uint8_t t = ZZA6_Template.toInt();
  if (t > 9){
    t = 0;
  }
  if (DPL_side[5] == 1){
    if (TPL_side[t] == 0 && TPL_side[t+1] == 1){
      t = t+1;
    }
  }

  DMUX(5);
  u8g2_uint_t x;
  disp.firstPage();
  if (TPL_invert[t] == 1){
    disp.sendF("c", 0x0a7);
  }
  else {
    disp.sendF("c", 0x0a6);
  }
  //disp.setContrast(DPL_contrast[5]);
  // Message
  // *** Message only, in the middle of the display ***
  if (TPL_6scroll[t] != 1){
    if (ZZA6_Message.length() > 1){
      disp.setFont(fontno[TPL_6font[t]]);
      // Switch to narrow font for longer messages
      if (disp.getUTF8Width(ZZA6_Message.c_str()) > TPL_6maxwidth[t]){
        disp.setFont(fontno[TPL_6font2[t]]);
      }
      disp.setFontMode(TPL_6fontmode[t]);
      disp.setDrawColor(TPL_6drawcolor[t]);
      
      disp.setCursor(TPL_6posx[t] + (TPL_6maxwidth[t] / 2) - (disp.getUTF8Width(ZZA6_Message.c_str()) / 2), TPL_6posy[t]);
      disp.print(ZZA6_Message);
    }
  }
  else {
    // *** Scrolling message on top ***
    if (ZZA6_Message.length() > 1){
      disp.setFont(fontno[TPL_6font[t]]);
      width6 = disp.getUTF8Width(ZZA6_MessageLoop.c_str());
      disp.setFontMode(TPL_6fontmode[t]);
      disp.setDrawColor(TPL_6drawcolor[t]);
      if (TPL_6boxh[t] > 0){
        disp.drawBox(TPL_6boxx[t], TPL_6boxy[t], TPL_6boxw[t], TPL_6boxh[t]);
        disp.setFontMode(TPL_6fontmode2[t]);
        disp.setDrawColor(TPL_6drawcolor2[t]);
        disp.drawBox(TPL_6box2x[t], TPL_6box2y[t], TPL_6box2w[t], TPL_6box2h[t]);
      }
      x = offset6;
      do {
        disp.drawUTF8(x, 8, ZZA6_MessageLoop.c_str());
        x += width6;
      } while( x < config.DISPWIDTH);
    }
  }
  disp.setFontMode(TPL_6fontmode[t]);
  disp.setDrawColor(TPL_6drawcolor[t]);

  // Station
  disp.setFont(fontno[TPL_0font[t]]);
  // Switch to narrow font for longer station names
  if (disp.getUTF8Width(ZZA6_Station.c_str()) > TPL_0maxwidth[t]){
    disp.setFont(fontno[TPL_0font2[t]]);
  }
  disp.drawUTF8((config.DISPWIDTH / 2) - (disp.getUTF8Width(ZZA6_Station.c_str()) / 2) + TPL_0posx[t], TPL_0posy[t], ZZA6_Station.c_str());

  // Track
  disp.setFont(fontno[TPL_1font[t]]);
  disp.setCursor(TPL_1posx[t],TPL_1posy[t]);
  disp.print(ZZA6_Track);

  // Destination
  disp.setFont(fontno[TPL_2font[t]]);
  // Switch to narrow font for longer destination names
  if (disp.getUTF8Width(ZZA6_Destination.c_str()) > TPL_2maxwidth[t]){
    disp.setFont(fontno[TPL_2font2[t]]);
  }
  disp.drawUTF8(TPL_2posx[t], TPL_2posy[t], ZZA6_Destination.c_str());

  // Departure
  disp.setFont(fontno[TPL_3font[t]]);
  //disp.setCursor(disp.getDisplayWidth() - disp.getUTF8Width(ZZA6_Departure.c_str()), TPL_3posy[t]);
  disp.setCursor(TPL_3posx[t], TPL_3posy[t]);
  disp.print(ZZA6_Departure);

  // Train
  disp.setFont(fontno[TPL_4font[t]]);
  //disp.setCursor(disp.getDisplayWidth() - disp.getUTF8Width(ZZA6_Train.c_str()), TPL_4posy[t]);
  disp.setCursor(TPL_4posx[t], TPL_4posy[t]);
  disp.print(ZZA6_Train);

  // Logo
  if ( ZZA6_Type != "" ) {
    switchLogo(t, ZZA6_Type);
  }
  
  disp.nextPage();

  offset6-=1;
  if ( (u8g2_uint_t)offset6 < (u8g2_uint_t)-width6 )
    offset6 = 0;
}


// *** Write to Display 7 ***
void send2display7(void)
{ 
  // Template number
  uint8_t t = ZZA7_Template.toInt();
  if (t > 9){
    t = 0;
  }
  if (DPL_side[6] == 1){
    if (TPL_side[t] == 0 && TPL_side[t+1] == 1){
      t = t+1;
    }
  }

  DMUX(6);
  u8g2_uint_t x;
  disp.firstPage();
  if (TPL_invert[t] == 1){
    disp.sendF("c", 0x0a7);
  }
  else {
    disp.sendF("c", 0x0a6);
  }
  //disp.setContrast(DPL_contrast[6]);
  // Message
  // *** Message only, in the middle of the display ***
  if (TPL_6scroll[t] != 1){
    if (ZZA7_Message.length() > 1){
      disp.setFont(fontno[TPL_6font[t]]);
      // Switch to narrow font for longer messages
      if (disp.getUTF8Width(ZZA7_Message.c_str()) > TPL_6maxwidth[t]){
        disp.setFont(fontno[TPL_6font2[t]]);
      }
      disp.setFontMode(TPL_6fontmode[t]);
      disp.setDrawColor(TPL_6drawcolor[t]);
      
      disp.setCursor(TPL_6posx[t] + (TPL_6maxwidth[t] / 2) - (disp.getUTF8Width(ZZA7_Message.c_str()) / 2), TPL_6posy[t]);
      disp.print(ZZA7_Message);
    }
  }
  else {
    // *** Scrolling message on top ***
    if (ZZA7_Message.length() > 1){
      disp.setFont(fontno[TPL_6font[t]]);
      width7 = disp.getUTF8Width(ZZA7_MessageLoop.c_str());
      disp.setFontMode(TPL_6fontmode[t]);
      disp.setDrawColor(TPL_6drawcolor[t]);
      if (TPL_6boxh[t] > 0){
        disp.drawBox(TPL_6boxx[t], TPL_6boxy[t], TPL_6boxw[t], TPL_6boxh[t]);
        disp.setFontMode(TPL_6fontmode2[t]);
        disp.setDrawColor(TPL_6drawcolor2[t]);
        disp.drawBox(TPL_6box2x[t], TPL_6box2y[t], TPL_6box2w[t], TPL_6box2h[t]);
      }
      x = offset7;
      do {
        disp.drawUTF8(x, 8, ZZA7_MessageLoop.c_str());
        x += width7;
      } while( x < config.DISPWIDTH);
    }
  }
  disp.setFontMode(TPL_6fontmode[t]);
  disp.setDrawColor(TPL_6drawcolor[t]);

  // Station
  disp.setFont(fontno[TPL_0font[t]]);
  // Switch to narrow font for longer station names
  if (disp.getUTF8Width(ZZA7_Station.c_str()) > TPL_0maxwidth[t]){
    disp.setFont(fontno[TPL_0font2[t]]);
  }
  disp.drawUTF8((config.DISPWIDTH / 2) - (disp.getUTF8Width(ZZA7_Station.c_str()) / 2) + TPL_0posx[t], TPL_0posy[t], ZZA7_Station.c_str());

  // Track
  disp.setFont(fontno[TPL_1font[t]]);
  disp.setCursor(TPL_1posx[t],TPL_1posy[t]);
  disp.print(ZZA7_Track);

  // Destination
  disp.setFont(fontno[TPL_2font[t]]);
  // Switch to narrow font for longer destination names
  if (disp.getUTF8Width(ZZA7_Destination.c_str()) > TPL_2maxwidth[t]){
    disp.setFont(fontno[TPL_2font2[t]]);
  }
  disp.drawUTF8(TPL_2posx[t], TPL_2posy[t], ZZA7_Destination.c_str());

  // Departure
  disp.setFont(fontno[TPL_3font[t]]);
  //disp.setCursor(disp.getDisplayWidth() - disp.getUTF8Width(ZZA7_Departure.c_str()), TPL_3posy[t]);
  disp.setCursor(TPL_3posx[t], TPL_3posy[t]);
  disp.print(ZZA7_Departure);

  // Train
  disp.setFont(fontno[TPL_4font[t]]);
  //disp.setCursor(disp.getDisplayWidth() - disp.getUTF8Width(ZZA7_Train.c_str()), TPL_4posy[t]);
  disp.setCursor(TPL_4posx[t], TPL_4posy[t]);
  disp.print(ZZA7_Train);

  // Logo
  if ( ZZA7_Type != "" ) {
    switchLogo(t, ZZA7_Type);
  }

  disp.nextPage();

  offset7-=1;
  if ( (u8g2_uint_t)offset7 < (u8g2_uint_t)-width7 )
    offset7 = 0;
}


// *** Write to Display 8 ***
void send2display8(void)
{ 
  // Template number
  uint8_t t = ZZA8_Template.toInt();
  if (t > 9){
    t = 0;
  }
  if (DPL_side[7] == 1){
    if (TPL_side[t] == 0 && TPL_side[t+1] == 1){
      t = t+1;
    }
  }

  DMUX(7);
  u8g2_uint_t x;
  disp.firstPage();
  if (TPL_invert[t] == 1){
    disp.sendF("c", 0x0a7);
  }
  else {
    disp.sendF("c", 0x0a6);
  }
  //disp.setContrast(DPL_contrast[7]);
  // Message
  // *** Message only, in the middle of the display ***
  if (TPL_6scroll[t] != 1){
    if (ZZA8_Message.length() > 1){
      disp.setFont(fontno[TPL_6font[t]]);
      // Switch to narrow font for longer messages
      if (disp.getUTF8Width(ZZA8_Message.c_str()) > TPL_6maxwidth[t]){
        disp.setFont(fontno[TPL_6font2[t]]);
      }
      disp.setFontMode(TPL_6fontmode[t]);
      disp.setDrawColor(TPL_6drawcolor[t]);
      
      disp.setCursor(TPL_6posx[t] + (TPL_6maxwidth[t] / 2) - (disp.getUTF8Width(ZZA8_Message.c_str()) / 2), TPL_6posy[t]);
      disp.print(ZZA8_Message);
    }
  }
  else {
    // *** Scrolling message on top ***
    if (ZZA8_Message.length() > 1){
      disp.setFont(fontno[TPL_6font[t]]);
      width8 = disp.getUTF8Width(ZZA8_MessageLoop.c_str());
      disp.setFontMode(TPL_6fontmode[t]);
      disp.setDrawColor(TPL_6drawcolor[t]);
      if (TPL_6boxh[t] > 0){
        disp.drawBox(TPL_6boxx[t], TPL_6boxy[t], TPL_6boxw[t], TPL_6boxh[t]);
        disp.setFontMode(TPL_6fontmode2[t]);
        disp.setDrawColor(TPL_6drawcolor2[t]);
        disp.drawBox(TPL_6box2x[t], TPL_6box2y[t], TPL_6box2w[t], TPL_6box2h[t]);
      }
      x = offset8;
      do {
        disp.drawUTF8(x, 8, ZZA8_MessageLoop.c_str());
        x += width8;
      } while( x < config.DISPWIDTH);
    }
  }
  disp.setFontMode(TPL_6fontmode[t]);
  disp.setDrawColor(TPL_6drawcolor[t]);

  // Station
  disp.setFont(fontno[TPL_0font[t]]);
  // Switch to narrow font for longer station names
  if (disp.getUTF8Width(ZZA8_Station.c_str()) > TPL_0maxwidth[t]){
    disp.setFont(fontno[TPL_0font2[t]]);
  }
  disp.drawUTF8((config.DISPWIDTH / 2) - (disp.getUTF8Width(ZZA8_Station.c_str()) / 2) + TPL_0posx[t], TPL_0posy[t], ZZA8_Station.c_str());

  // Track
  disp.setFont(fontno[TPL_1font[t]]);
  disp.setCursor(TPL_1posx[t],TPL_1posy[t]);
  disp.print(ZZA8_Track);

  // Destination
  disp.setFont(fontno[TPL_2font[t]]);
  // Switch to narrow font for longer destination names
  if (disp.getUTF8Width(ZZA8_Destination.c_str()) > TPL_2maxwidth[t]){
    disp.setFont(fontno[TPL_2font2[t]]);
  }
  disp.drawUTF8(TPL_2posx[t], TPL_2posy[t], ZZA8_Destination.c_str());

  // Departure
  disp.setFont(fontno[TPL_3font[t]]);
  //disp.setCursor(disp.getDisplayWidth() - disp.getUTF8Width(ZZA8_Departure.c_str()), TPL_3posy[t]);
  disp.setCursor(TPL_3posx[t], TPL_3posy[t]);
  disp.print(ZZA8_Departure);

  // Train
  disp.setFont(fontno[TPL_4font[t]]);
  //disp.setCursor(disp.getDisplayWidth() - disp.getUTF8Width(ZZA8_Train.c_str()), TPL_4posy[t]);
  disp.setCursor(TPL_4posx[t], TPL_4posy[t]);
  disp.print(ZZA8_Train);

  // Logo
  if ( ZZA8_Type != "" ) {
    switchLogo(t, ZZA8_Type);
  }
  disp.nextPage();

  offset8-=1;
  if ( (u8g2_uint_t)offset8 < (u8g2_uint_t)-width8 )
    offset8 = 0;
}


// Display a logo based on TrainType field
void switchLogo(uint8_t t, String ZZA_Type)
{
  if (ZZA_Type == logoId[0]){
    disp.drawXBM( TPL_5logox[t], TPL_5logoy[t], logow[0], logoh[0], logo0);
  }
  else if (ZZA_Type == logoId[1]){
    disp.drawXBM( TPL_5logox[t], TPL_5logoy[t], logow[1], logoh[1], logo1);
  }
  else if (ZZA_Type == logoId[2]){
    disp.drawXBM( TPL_5logox[t], TPL_5logoy[t], logow[2], logoh[2], logo2);
  }
  else if (ZZA_Type == logoId[3]){
    disp.drawXBM( TPL_5logox[t], TPL_5logoy[t], logow[3], logoh[3], logo3);
  }
  else if (ZZA_Type == logoId[4]){
    disp.drawXBM( TPL_5logox[t], TPL_5logoy[t], logow[4], logoh[4], logo4);
  }
  else if (ZZA_Type == logoId[5]){
    disp.drawXBM( TPL_5logox[t], TPL_5logoy[t], logow[5], logoh[5], logo5);
  }
  else if (ZZA_Type == logoId[6]){
    disp.drawXBM( TPL_5logox[t], TPL_5logoy[t], logow[6], logoh[6], logo6);
  }
  else if (ZZA_Type == logoId[7]){
    disp.drawXBM( TPL_5logox[t], TPL_5logoy[t], logow[7], logoh[7], logo7);
  }
  else if (ZZA_Type == logoId[8]){
    disp.drawXBM( TPL_5logox[t], TPL_5logoy[t], logow[8], logoh[8], logo8);
  }
  else if (ZZA_Type == logoId[9]){
    disp.drawXBM( TPL_5logox[t], TPL_5logoy[t], logow[9], logoh[9], logo9);
  }
  else if (ZZA_Type == logoId[10]){
    disp.drawXBM( TPL_5logox[t], TPL_5logoy[t], logow[10], logoh[10], logo10);
  }
  else if (ZZA_Type == logoId[11]){
    disp.drawXBM( TPL_5logox[t], TPL_5logoy[t], logow[11], logoh[11], logo11);
  }
  else if (ZZA_Type == logoId[12]){
    disp.drawXBM( TPL_5logox[t], TPL_5logoy[t], logow[12], logoh[12], logo12);
  }
  else if (ZZA_Type == logoId[13]){
    disp.drawXBM( TPL_5logox[t], TPL_5logoy[t], logow[13], logoh[13], logo13);
  }
  else if (ZZA_Type == logoId[14]){
    disp.drawXBM( TPL_5logox[t], TPL_5logoy[t], logow[14], logoh[14], logo14);
  }
  else if (ZZA_Type == logoId[15]){
    disp.drawXBM( TPL_5logox[t], TPL_5logoy[t], logow[15], logoh[15], logo15);
  }
  else if (ZZA_Type == logoId[16]){
    disp.drawXBM( TPL_5logox[t], TPL_5logoy[t], logow[16], logoh[16], logo16);
  }
  else if (ZZA_Type == logoId[17]){
    disp.drawXBM( TPL_5logox[t], TPL_5logoy[t], logow[17], logoh[17], logo17);
  }
  else if (ZZA_Type == logoId[18]){
    disp.drawXBM( TPL_5logox[t], TPL_5logoy[t], logow[18], logoh[18], logo18);
  }
  else if (ZZA_Type == logoId[19]){
    disp.drawXBM( TPL_5logox[t], TPL_5logoy[t], logow[19], logoh[19], logo19);
  }
  
  /*switch (i){ 
    case 1: disp.drawXBM( TPL_6logox[t], TPL_6logoy[t], TPL_6logow[t], TPL_6logoh[t], logo1);
            break;
    case 2: disp.drawXBM( TPL_6logox[t], TPL_6logoy[t], TPL_6logow[t], TPL_6logoh[t], logo2);
            break;
    case 3: disp.drawXBM( TPL_6logox[t], TPL_6logoy[t], TPL_6logow[t], TPL_6logoh[t], logo3);
            break;
    case 4: disp.drawXBM( TPL_6logox[t], TPL_6logoy[t], TPL_6logow[t], TPL_6logoh[t], logo4);
            break;
    case 5: disp.drawXBM( TPL_6logox[t], TPL_6logoy[t], TPL_6logow[t], TPL_6logoh[t], logo5);
            break;
    case 6: disp.drawXBM( TPL_6logox[t], TPL_6logoy[t], TPL_6logow[t], TPL_6logoh[t], logo6);
            break;
    case 7: disp.drawXBM( TPL_6logox[t], TPL_6logoy[t], TPL_6logow[t], TPL_6logoh[t], logo7);
            break;
    case 8: disp.drawXBM( TPL_6logox[t], TPL_6logoy[t], TPL_6logow[t], TPL_6logoh[t], logo8);
            break;
    case 9: disp.drawXBM( TPL_6logox[t], TPL_6logoy[t], TPL_6logow[t], TPL_6logoh[t], logo9);
  }
  //return logo;
  */
}


// Enable ScreenSaver for all displays
void screenSaver(int s){
  for (uint8_t i = 0; i < config.NUMDISP; i++)
  {
    if (config.MUX == 112){
      DMUX(i);
    }
    else {
      disp.sendBuffer();
    }    
    //DMUX(i);
    disp.setPowerSave(s);
    // AEh : Display OFF
    // AFh : Display ON
    //disp.sendF("c", 0x0ae);
    //disp.sendF("c", 0x0af);
  }
}


// Write display buffer/screenshot to serial out
void printBuffer(){
  Serial.println("\nScreenshot of display 1 as XBM image\n");
  disp.writeBufferXBM(Serial);      // Write XBM image to serial out
  Serial.println();
}


void updateTime() {
  time(&now);                         // read the current time
  localtime_r(&now, &tm);             // update the structure tm with the current time

  if (millis() - lastNTP < 20000){
    // Format time, add leading 0's
    ntptime = ("0" + String(tm.tm_hour)).substring( String(tm.tm_hour).length() - 1,  String(tm.tm_hour).length() +1) + ":" + ("0" + String(tm.tm_min)).substring( String(tm.tm_min).length() - 1,  String(tm.tm_min).length() +1);
    ntpdate = ("0" + String(tm.tm_mday)).substring( String(tm.tm_mday).length() - 1,  String(tm.tm_mday).length() +1) + "." + ("0" + String(tm.tm_mon + 1)).substring( String(tm.tm_mon + 1).length() - 1, String(tm.tm_mon + 1).length() + 1) + "." + String(tm.tm_year + 1900);
    lastNTP = millis();
  }
  else {
    ntptime = "no NTP data";
  }
  //Serial.print(ntptime);
  //if (tm.tm_isdst == 1)                  // Daylight Saving Time flag
  //  Serial.println("\t  Daylight Saving Time");
  //else
  //  Serial.println("\t  Standard Time");
}


void runCmd(){
  ZZA1_Message = ZZA1_MessageO;
  ZZA1_Message.replace("{ntptime}", ntptime);
  ZZA1_Message.replace("{ntpdate}", ntpdate);
  ZZA1_Message.replace("{rrtime}", rrtime);
  ZZA1_Message.replace("{rrdate}", rrdate);
  ZZA1_MessageLoop = " +++ " + ZZA1_Message;
  width1 = disp.getUTF8Width(ZZA1_MessageLoop.c_str());

  ZZA2_Message = ZZA2_MessageO;
  ZZA2_Message.replace("{ntptime}", ntptime);
  ZZA2_Message.replace("{ntpdate}", ntpdate);
  ZZA2_Message.replace("{rrtime}", rrtime);
  ZZA2_Message.replace("{rrdate}", rrdate);
  ZZA2_MessageLoop = " +++ " + ZZA2_Message;
  width2 = disp.getUTF8Width(ZZA2_MessageLoop.c_str());

  ZZA3_Message = ZZA3_MessageO;
  ZZA3_Message.replace("{ntptime}", ntptime);
  ZZA3_Message.replace("{ntpdate}", ntpdate);
  ZZA3_Message.replace("{rrtime}", rrtime);
  ZZA3_Message.replace("{rrdate}", rrdate);
  ZZA3_MessageLoop = " +++ " + ZZA3_Message;
  width3 = disp.getUTF8Width(ZZA3_MessageLoop.c_str());

  ZZA4_Message = ZZA4_MessageO;
  ZZA4_Message.replace("{ntptime}", ntptime);
  ZZA4_Message.replace("{ntpdate}", ntpdate);
  ZZA4_Message.replace("{rrtime}", rrtime);
  ZZA4_Message.replace("{rrdate}", rrdate);
  ZZA4_MessageLoop = " +++ " + ZZA4_Message;
  width4 = disp.getUTF8Width(ZZA4_MessageLoop.c_str());

  ZZA5_Message = ZZA5_MessageO;
  ZZA5_Message.replace("{ntptime}", ntptime);
  ZZA5_Message.replace("{ntpdate}", ntpdate);
  ZZA5_Message.replace("{rrtime}", rrtime);
  ZZA5_Message.replace("{rrdate}", rrdate);
  ZZA5_MessageLoop = " +++ " + ZZA5_Message;
  width5 = disp.getUTF8Width(ZZA5_MessageLoop.c_str());

  ZZA6_Message = ZZA6_MessageO;
  ZZA6_Message.replace("{ntptime}", ntptime);
  ZZA6_Message.replace("{ntpdate}", ntpdate);
  ZZA6_Message.replace("{rrtime}", rrtime);
  ZZA6_Message.replace("{rrdate}", rrdate);
  ZZA6_MessageLoop = " +++ " + ZZA6_Message;
  width6 = disp.getUTF8Width(ZZA6_MessageLoop.c_str());

  ZZA7_Message = ZZA7_MessageO;
  ZZA7_Message.replace("{ntptime}", ntptime);
  ZZA7_Message.replace("{ntpdate}", ntpdate);
  ZZA7_Message.replace("{rrtime}", rrtime);
  ZZA7_Message.replace("{rrdate}", rrdate);
  ZZA7_MessageLoop = " +++ " + ZZA7_Message;
  width7 = disp.getUTF8Width(ZZA7_MessageLoop.c_str());

  ZZA8_Message = ZZA8_MessageO;
  ZZA8_Message.replace("{ntptime}", ntptime);
  ZZA8_Message.replace("{ntpdate}", ntpdate);
  ZZA8_Message.replace("{rrtime}", rrtime);
  ZZA8_Message.replace("{rrdate}", rrdate);
  ZZA8_MessageLoop = " +++ " + ZZA8_Message;
  width8 = disp.getUTF8Width(ZZA8_MessageLoop.c_str());
}


// This function is called when WIFI and MQTT are connected
void onConnectionEstablished()
{
  // Subscribe MQTT client to topic: "rmdtest/connectiontest" to test the broker connection when debug messages are enabled
  if (config.MQTT_DEBUG == 1){
    client.subscribe("rmdtest/connectiontest", [](const String & payload0){
      if (payload0 == "rmdtest"){
        Serial.println((String)"\nMQTT broker successfully connected at " + config.MQTT_IP + ":" + config.MQTT_PORT + "\n");
      }
      else {
        Serial.println((String)"\nNO connection to MQTT broker at " + config.MQTT_IP + ":" + config.MQTT_PORT + " !!!\n");
      }
    }, 1);
    Serial.println(F("Send test message to broker:"));
    client.publish("rmdtest/connectiontest", "rmdtest");
  }
  Serial.print(F("\nOTA Ready -> Port: "));
  Serial.print(config.OTA_HOSTNAME);
  Serial.print(F(" at "));
  Serial.println(WiFi.localIP());

  Serial.print(F("\nFOR CONFIGURATION OPEN: http://"));
  Serial.println(config.WIFI_DEVICENAME);
  Serial.println(F("                          or "));
  Serial.print(F("                        http://"));
  Serial.println(WiFi.localIP());

  // Subscribe MQTT client to topic: "rocrail/service/info/clock" to receive Rocrail time
  //client.subscribe("rocrail/service/info/clock", [](const String & payload1) {
  client.subscribe(config.MQTT_TOPIC1, [](const String & payload1) {
    //Serial.println(payload1);
    // <clock divider="1" hour="18" minute="40" wday="5" mday="12" month="2" year="2021" time="1613151626" temp="20" bri="255" lux="0" pressure="0" humidity="0" cmd="sync"/>
    String h = payload1.substring(payload1.indexOf("hour") + 6, payload1.indexOf("minute") - 2);
    if (h.length() < 2){
      h = "0" + h;
    }
    String m = payload1.substring(payload1.indexOf("minute") + 8, payload1.indexOf("wday") - 2);
    if (m.length() < 2){
      m = "0" + m;
    }
    uint8_t w = (payload1.substring(payload1.indexOf("wday") + 6, payload1.indexOf("mday") - 2)).toInt();
    String wd = "";
    switch (w) {
      case 1: wd = "Mo";
              break;
      case 2: wd = "Di";
              break;
      case 3: wd = "Mi";
              break;
      case 4: wd = "Do";
              break;
      case 5: wd = "Fr";
              break;
      case 6: wd = "Sa";
              break;
      case 7: wd = "So";
              break;
    }
    String d = payload1.substring(payload1.indexOf("mday") + 6, payload1.indexOf("month") - 2);
    if (d.length() < 2){
      d = "0" + d;
    }
    String mo = payload1.substring(payload1.indexOf("month") + 7, payload1.indexOf("year") - 2);
    if (mo.length() < 2){
      mo = "0" + mo;
    }
    String y = payload1.substring(payload1.indexOf("year") + 6, payload1.indexOf("time") - 2);
    
    rrtime = h + ":" + m;
    rrdate = d + "." + mo + "." + y;
    runCmd();
  }, 1);

  // Subscribe MQTT client to topic: "rocrail/service/info/tx" to receive messages sent by Rocrail text fields or other MQTT sources
  client.subscribe(config.MQTT_TOPIC2, [](const String & payload2) {
    String pld = payload2.substring(payload2.indexOf("ZZAMSG"), payload2.length() - 4);
    if (config.MQTT_DEBUG == 1){
      Serial.println("Received message: " + payload2);
      Serial.println("Received payload:  " + pld);
      //Serial.println(strlen(config.MQTT_DELIMITER));
      //Serial.println(config.MQTT_DELIMITER);
    }
    if (pld.substring(0, 6) == "ZZAMSG"){
      if (strlen(config.MQTT_DELIMITER) > 0){
        pld.replace(String(config.MQTT_DELIMITER), "#");
      }
      if (config.MQTT_DEBUG == 1){
        Serial.println("Converted payload: " + pld);
      }
      uint8_t start01 = pld.indexOf("ZZAMSG#") + 7;                         // ZZAMSG identifier
      uint8_t start02 = start01 + 1 + pld.substring(start01).indexOf("#");  // Target Displays as defined in config.h e.g. D01-D08
      uint8_t start03 = start02 + 1 + pld.substring(start02).indexOf("#");  // Template T0-T9
      uint8_t start04 = start03 + 1 + pld.substring(start03).indexOf("#");  // Station Name
      uint8_t start05 = start04 + 1 + pld.substring(start04).indexOf("#");  // Track Number
      uint8_t start06 = start05 + 1 + pld.substring(start05).indexOf("#");  // Destination Name
      uint8_t start07 = start06 + 1 + pld.substring(start06).indexOf("#");  // Departure Time
      uint8_t start08 = start07 + 1 + pld.substring(start07).indexOf("#");  // Train Number
      uint8_t start09 = start08 + 1 + pld.substring(start08).indexOf("#");  // Train Type e.g. ICE, IC, ...
      uint8_t start10 = start09 + 1 + pld.substring(start09).indexOf("#");  // Message Text


      // Deactivate / Reset ScreenSaver
      screenSaver(0);
      lastMsg = millis();

      // Display 1
      if (pld.substring(start01, start02 -1).indexOf(DPL_id[0]) != -1){
        ZZA1_Targets = pld.substring(start01, start02 -1);
        ZZA1_Template = pld.substring(start02 + 1, start03 -1);
        ZZA1_Station = pld.substring(start03, start04 -1);
        ZZA1_Track = pld.substring(start04, start05 -1);
        ZZA1_Destination = pld.substring(start05, start06 -1);
        ZZA1_Departure = pld.substring(start06, start07 -1);
        ZZA1_Train = pld.substring(start07, start08 -1);
        ZZA1_Type = pld.substring(start08, start09 -1);
        ZZA1_MessageO = pld.substring(start09, start10 -1);
        ZZA1_Message = ZZA1_MessageO;
        if (pld.indexOf("{") > 0){
          ZZA1_Message.replace("{ntptime}", ntptime);
          ZZA1_Message.replace("{ntpdate}", ntpdate);
          ZZA1_Message.replace("{rrtime}", rrtime);
          ZZA1_Message.replace("{rrdate}", rrdate);
        }
        ZZA1_MessageLoop = " +++ " + ZZA1_Message;
        width1 = disp.getUTF8Width(ZZA1_MessageLoop.c_str());
      }
      
      // Display 2
      if (pld.substring(start01, start02 -1).indexOf(DPL_id[1]) != -1){
        ZZA2_Targets = pld.substring(start01, start02 -1);
        ZZA2_Template = pld.substring(start02 + 1, start03 -1);
        ZZA2_Station = pld.substring(start03, start04 -1);
        ZZA2_Track = pld.substring(start04, start05 -1);
        ZZA2_Destination = pld.substring(start05, start06 -1);
        ZZA2_Departure = pld.substring(start06, start07 -1);
        ZZA2_Train = pld.substring(start07, start08 -1);
        ZZA2_Type = pld.substring(start08, start09 -1);
        ZZA2_MessageO = pld.substring(start09, start10 -1);
        ZZA2_Message = ZZA2_MessageO;
        if (pld.indexOf("{") > 0){
          ZZA2_Message.replace("{ntptime}", ntptime);
          ZZA2_Message.replace("{ntpdate}", ntpdate);
          ZZA2_Message.replace("{rrtime}", rrtime);
          ZZA2_Message.replace("{rrdate}", rrdate);
        }        
        ZZA2_MessageLoop = " +++ " + ZZA2_Message;
        width2 = disp.getUTF8Width(ZZA2_MessageLoop.c_str());
      }

      // Display 3
      if (pld.substring(start01, start02 -1).indexOf(DPL_id[2]) != -1){
        ZZA3_Targets = pld.substring(start01, start02 -1);
        ZZA3_Template = pld.substring(start02 + 1, start03 -1);
        ZZA3_Station = pld.substring(start03, start04 -1);
        ZZA3_Track = pld.substring(start04, start05 -1);
        ZZA3_Destination = pld.substring(start05, start06 -1);
        ZZA3_Departure = pld.substring(start06, start07 -1);
        ZZA3_Train = pld.substring(start07, start08 -1);
        ZZA3_Type = pld.substring(start08, start09 -1);
        ZZA3_MessageO = pld.substring(start09, start10 -1);
        ZZA3_Message = ZZA3_MessageO;
        if (pld.indexOf("{") > 0){
          ZZA3_Message.replace("{ntptime}", ntptime);
          ZZA3_Message.replace("{ntpdate}", ntpdate);
          ZZA3_Message.replace("{rrtime}", rrtime);
          ZZA3_Message.replace("{rrdate}", rrdate);
        }
        ZZA3_MessageLoop = " +++ " + ZZA3_Message;
        width3 = disp.getUTF8Width(ZZA3_MessageLoop.c_str());
      }

      // Display 4
      if (pld.substring(start01, start02 -1).indexOf(DPL_id[3]) != -1){
        ZZA4_Targets = pld.substring(start01, start02 -1);
        ZZA4_Template = pld.substring(start02 + 1, start03 -1);
        ZZA4_Station = pld.substring(start03, start04 -1);
        ZZA4_Track = pld.substring(start04, start05 -1);
        ZZA4_Destination = pld.substring(start05, start06 -1);
        ZZA4_Departure = pld.substring(start06, start07 -1);
        ZZA4_Train = pld.substring(start07, start08 -1);
        ZZA4_Type = pld.substring(start08, start09 -1);
        ZZA4_MessageO = pld.substring(start09, start10 -1);
        ZZA4_Message = ZZA4_MessageO;
        if (pld.indexOf("{") > 0){
          ZZA4_Message.replace("{ntptime}", ntptime);
          ZZA4_Message.replace("{ntpdate}", ntpdate);
          ZZA4_Message.replace("{rrtime}", rrtime);
          ZZA4_Message.replace("{rrdate}", rrdate);
        }        
        ZZA4_MessageLoop = " +++ " + ZZA4_Message;
        width4 = disp.getUTF8Width(ZZA4_MessageLoop.c_str());
      }

      // Display 5
      if (pld.substring(start01, start02 -1).indexOf(DPL_id[4]) != -1){
        ZZA5_Targets = pld.substring(start01, start02 -1);
        ZZA5_Template = pld.substring(start02 + 1, start03 -1);
        ZZA5_Station = pld.substring(start03, start04 -1);
        ZZA5_Track = pld.substring(start04, start05 -1);
        ZZA5_Destination = pld.substring(start05, start06 -1);
        ZZA5_Departure = pld.substring(start06, start07 -1);
        ZZA5_Train = pld.substring(start07, start08 -1);
        ZZA5_Type = pld.substring(start08, start09 -1);
        ZZA5_MessageO = pld.substring(start09, start10 -1);
        ZZA5_Message = ZZA5_MessageO;
        if (pld.indexOf("{") > 0){
          ZZA5_Message.replace("{ntptime}", ntptime);
          ZZA5_Message.replace("{ntpdate}", ntpdate);
          ZZA5_Message.replace("{rrtime}", rrtime);
          ZZA5_Message.replace("{rrdate}", rrdate);
        }        
        ZZA5_MessageLoop = " +++ " + ZZA5_Message;
        width5 = disp.getUTF8Width(ZZA5_MessageLoop.c_str());
      }

      // Display 6
      if (pld.substring(start01, start02 -1).indexOf(DPL_id[5]) != -1){
        ZZA6_Targets = pld.substring(start01, start02 -1);
        ZZA6_Template = pld.substring(start02 + 1, start03 -1);
        ZZA6_Station = pld.substring(start03, start04 -1);
        ZZA6_Track = pld.substring(start04, start05 -1);
        ZZA6_Destination = pld.substring(start05, start06 -1);
        ZZA6_Departure = pld.substring(start06, start07 -1);
        ZZA6_Train = pld.substring(start07, start08 -1);
        ZZA6_Type = pld.substring(start08, start09 -1);
        ZZA6_MessageO = pld.substring(start09, start10 -1);
        ZZA6_Message = ZZA6_MessageO;
        if (pld.indexOf("{") > 0){
          ZZA6_Message.replace("{ntptime}", ntptime);
          ZZA6_Message.replace("{ntpdate}", ntpdate);
          ZZA6_Message.replace("{rrtime}", rrtime);
          ZZA6_Message.replace("{rrdate}", rrdate);
        }        
        ZZA6_MessageLoop = " +++ " + ZZA6_Message;
        width6 = disp.getUTF8Width(ZZA6_MessageLoop.c_str());
      }

      // Display 7
      if (pld.substring(start01, start02 -1).indexOf(DPL_id[6]) != -1){
        ZZA7_Targets = pld.substring(start01, start02 -1);
        ZZA7_Template = pld.substring(start02 + 1, start03 -1);
        ZZA7_Station = pld.substring(start03, start04 -1);
        ZZA7_Track = pld.substring(start04, start05 -1);
        ZZA7_Destination = pld.substring(start05, start06 -1);
        ZZA7_Departure = pld.substring(start06, start07 -1);
        ZZA7_Train = pld.substring(start07, start08 -1);
        ZZA7_Type = pld.substring(start08, start09 -1);
        ZZA7_MessageO = pld.substring(start09, start10 -1);
        ZZA7_Message = ZZA7_MessageO;
        if (pld.indexOf("{") > 0){
          ZZA7_Message.replace("{ntptime}", ntptime);
          ZZA7_Message.replace("{ntpdate}", ntpdate);
          ZZA7_Message.replace("{rrtime}", rrtime);
          ZZA7_Message.replace("{rrdate}", rrdate);
        }        
        ZZA7_MessageLoop = " +++ " + ZZA7_Message;
        width7 = disp.getUTF8Width(ZZA7_MessageLoop.c_str());
      }

      // Display 8
      if (pld.substring(start01, start02 -1).indexOf(DPL_id[7]) != -1){
        ZZA8_Targets = pld.substring(start01, start02 -1);
        ZZA8_Template = pld.substring(start02 + 1, start03 -1);
        ZZA8_Station = pld.substring(start03, start04 -1);
        ZZA8_Track = pld.substring(start04, start05 -1);
        ZZA8_Destination = pld.substring(start05, start06 -1);
        ZZA8_Departure = pld.substring(start06, start07 -1);
        ZZA8_Train = pld.substring(start07, start08 -1);
        ZZA8_Type = pld.substring(start08, start09 -1);
        ZZA8_MessageO = pld.substring(start09, start10 -1);
        ZZA8_Message = ZZA8_MessageO;
        if (pld.indexOf("{") > 0){
          ZZA8_Message.replace("{ntptime}", ntptime);
          ZZA8_Message.replace("{ntpdate}", ntpdate);
          ZZA8_Message.replace("{rrtime}", rrtime);
          ZZA8_Message.replace("{rrdate}", rrdate);
        }        
        ZZA8_MessageLoop = " +++ " + ZZA8_Message;
        width8 = disp.getUTF8Width(ZZA8_MessageLoop.c_str());
      }
    }
    else {
      Serial.println(F("Error - No valid ZZAMSG Message"));
      Serial.println(pld);
    }
  }, 1);
}


// Main loop
void loop()
{
  client.loop();             // WIFI, MQTT
  
  ArduinoOTA.handle();       // OTA
  
  webserver.handleClient();  // WEBSERVER handling of incoming requests

  updateTime();              // NTP update time information

  if ( millis() > (lastMsg + (config.SCREENSAVER * 60000)) && config.SCREENSAVER > 0 ){
    screenSaver(1);          // Activate ScreenSaver
  }

  ts.execute();              // TaskScheduler updating the displays
}
