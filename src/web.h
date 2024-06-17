// Roc-MQTT-Display WEBSERVER
// Version 1.13
// Copyright (c) 2020-2024 Christian Heinrichs. All rights reserved.
// https://github.com/chrisweather/RocMQTTdisplay

#ifndef WEB_H
#define WEB_H
#include <FS.h>
#include <LittleFS.h>            // LittleFS file system https://github.com/esp8266/Arduino/tree/master/libraries/LittleFS
#include "config.h"              // Roc-MQTT-Display configuration file
#if defined(ESP8266)             // ESP8266
#include <ESP8266WebServer.h>    //
ESP8266WebServer webserver(80);  //
#elif defined(ESP32)             // ESP32
#include <WebServer.h>           //
WebServer webserver(80);         //
#else
#error "This software only works with ESP32 or ESP8266 boards!"
#endif

String buf1 = "";
String buf2 = "";

// CSS
void loadCSS()
{
  File htmlCSS = LittleFS.open( "/rmd.css", "r" );
  webserver.streamFile( htmlCSS, "text/css" );
  htmlCSS.close();
}

// STATISTICS
String handleStats()
{
  #if defined(ESP8266)
    String tmt = "mo=ESP8266-"+String(ESP.getChipId())+"&me="+ESP.getFlashChipRealSize()+"&ma="+String(WiFi.macAddress())+"&ds="+config.DISPWIDTH+"x"+config.DISPHEIGHT+"&dn="+config.NUMDISP+"&ve=v"+config.VER;
  #elif defined(ESP32)
    String tmt = "mo="+String(ESP.getChipModel())+"&me="+ESP.getFlashChipSize()+"&ma="+String(WiFi.macAddress())+"&ds="+config.DISPWIDTH+"x"+config.DISPHEIGHT+"&dn="+config.NUMDISP+"&ve=v"+config.VER;
  #else
    String tmt = "mo=No Data";
  #endif
  return tmt;
}

// ROOT
void loadRoot()
{
  File htmlRoot = LittleFS.open( "/index.htm", "r" );
  buf1 = htmlRoot.readString();
  htmlRoot.close();
  buf1.replace("%VER%", config.VER);
  buf1.replace("%WIFI_DEVICENAME%", String(config.WIFI_DEVICENAME));
  buf1.replace("%STAT%", handleStats());
  webserver.setContentLength( buf1.length() );
  webserver.send( 200, "text/html", buf1 );
  buf1 = "";
}

// 404 - NotFound
void loadNotFound()
{
  File html404 = LittleFS.open( "/404.htm", "r" );
  webserver.streamFile( html404, "text/html" );
  html404.close();
}

// CONFIGURATION
void loadCfg()
{
  File htmlCfg = LittleFS.open( "/config.htm", "r" );
  buf1 = htmlCfg.readString();
  htmlCfg.close();
  buf1.replace("%WIFI_DEVICENAME%", String(config.WIFI_DEVICENAME));
  buf1.replace("%WIFI_RECONDELAY%", String(config.WIFI_RECONDELAY));
  buf1.replace("%OTA_HOSTNAME%", String(config.OTA_HOSTNAME));
  buf1.replace("%OTA_PORT%", String(config.OTA_PORT));
  buf1.replace("%NTP_SERVER%", String(config.NTP_SERVER));
  buf1.replace("%NTP_TZ%", String(config.NTP_TZ));
  buf1.replace("%MQTT_IP%", String(config.MQTT_IP));
  buf1.replace("%MQTT_PORT%", String(config.MQTT_PORT));
  buf1.replace("%MQTT_MSGSIZE%", String(config.MQTT_MSGSIZE));
  buf1.replace("%MQTT_KEEPALIVE%", String(config.MQTT_KEEPALIVE1));
  buf1.replace("%MQTT_RECONDELAY%", String(config.MQTT_RECONDELAY));
  buf1.replace("%MQTT_TOPIC1%", String(config.MQTT_TOPIC1));
  buf1.replace("%MQTT_TOPIC2%", String(config.MQTT_TOPIC2));
  buf1.replace("%MQTT_DELIMITER%", String(config.MQTT_DELIMITER));
  buf1.replace("%MQTT_DEBUG%", String(config.MQTT_DEBUG));
  buf1.replace("%MUX%", String(config.MUX));
  if (config.MUX < 16){
    buf1.replace("%MUXHEX%", String("0x0") + String(config.MUX, HEX));
  }
  else {
    buf1.replace("%MUXHEX%", String("0x") + String(config.MUX, HEX));
  }
  buf1.replace("%NUMDISP%", String(config.NUMDISP));
  buf1.replace("%DISPWIDTH%", String(config.DISPWIDTH));
  buf1.replace("%DISPHEIGHT%", String(config.DISPHEIGHT));
  buf1.replace("%DISPSIZE%", String(config.DISPWIDTH) + " x " + String(config.DISPHEIGHT));
  buf1.replace("%STARTDELAY%", String(config.STARTDELAY));
  buf1.replace("%UPDSPEED%", String(config.UPDSPEED));
  buf1.replace("%SCREENSAVER%", String(config.SCREENSAVER));
  buf1.replace("%PRINTBUF%", String(config.PRINTBUF));
  buf1.replace("%DPL_ID0%", String(DPL_id[0]));
  buf1.replace("%DPL_ID1%", String(DPL_id[1]));
  buf1.replace("%DPL_ID2%", String(DPL_id[2]));
  buf1.replace("%DPL_ID3%", String(DPL_id[3]));
  buf1.replace("%DPL_ID4%", String(DPL_id[4]));
  buf1.replace("%DPL_ID5%", String(DPL_id[5]));
  buf1.replace("%DPL_ID6%", String(DPL_id[6]));
  buf1.replace("%DPL_ID7%", String(DPL_id[7]));
  buf1.replace("%DPL_STATION00%", String(DPL_station[0]));
  buf1.replace("%DPL_STATION01%", String(DPL_station[1]));
  buf1.replace("%DPL_STATION02%", String(DPL_station[2]));
  buf1.replace("%DPL_STATION03%", String(DPL_station[3]));
  buf1.replace("%DPL_STATION04%", String(DPL_station[4]));
  buf1.replace("%DPL_STATION05%", String(DPL_station[5]));
  buf1.replace("%DPL_STATION06%", String(DPL_station[6]));
  buf1.replace("%DPL_STATION07%", String(DPL_station[7]));
  buf1.replace("%DPL_TRACK0%", String(DPL_track[0]));
  buf1.replace("%DPL_TRACK1%", String(DPL_track[1]));
  buf1.replace("%DPL_TRACK2%", String(DPL_track[2]));
  buf1.replace("%DPL_TRACK3%", String(DPL_track[3]));
  buf1.replace("%DPL_TRACK4%", String(DPL_track[4]));
  buf1.replace("%DPL_TRACK5%", String(DPL_track[5]));
  buf1.replace("%DPL_TRACK6%", String(DPL_track[6]));
  buf1.replace("%DPL_TRACK7%", String(DPL_track[7]));
  buf1.replace("%DPL_FLIP0%", String(DPL_flip[0]));
  buf1.replace("%DPL_FLIP1%", String(DPL_flip[1]));
  buf1.replace("%DPL_FLIP2%", String(DPL_flip[2]));
  buf1.replace("%DPL_FLIP3%", String(DPL_flip[3]));
  buf1.replace("%DPL_FLIP4%", String(DPL_flip[4]));
  buf1.replace("%DPL_FLIP5%", String(DPL_flip[5]));
  buf1.replace("%DPL_FLIP6%", String(DPL_flip[6]));
  buf1.replace("%DPL_FLIP7%", String(DPL_flip[7]));
  buf1.replace("%DPL_CONTRAST0%", String(DPL_contrast[0]));
  buf1.replace("%DPL_CONTRAST1%", String(DPL_contrast[1]));
  buf1.replace("%DPL_CONTRAST2%", String(DPL_contrast[2]));
  buf1.replace("%DPL_CONTRAST3%", String(DPL_contrast[3]));
  buf1.replace("%DPL_CONTRAST4%", String(DPL_contrast[4]));
  buf1.replace("%DPL_CONTRAST5%", String(DPL_contrast[5]));
  buf1.replace("%DPL_CONTRAST6%", String(DPL_contrast[6]));
  buf1.replace("%DPL_CONTRAST7%", String(DPL_contrast[7]));
  buf1.replace("%DPL_SIDE0%", String(DPL_side[0]));
  buf1.replace("%DPL_SIDE1%", String(DPL_side[1]));
  buf1.replace("%DPL_SIDE2%", String(DPL_side[2]));
  buf1.replace("%DPL_SIDE3%", String(DPL_side[3]));
  buf1.replace("%DPL_SIDE4%", String(DPL_side[4]));
  buf1.replace("%DPL_SIDE5%", String(DPL_side[5]));
  buf1.replace("%DPL_SIDE6%", String(DPL_side[6]));
  buf1.replace("%DPL_SIDE7%", String(DPL_side[7]));
  webserver.setContentLength( buf1.length() );
  webserver.send( 200, "text/html", buf1 );
  buf1 = "";
}

void handleCfgSubmit()
{
  if (webserver.args() > 0 ) {
    for ( uint8_t i = 0; i < webserver.args(); i++ ) {
      if (webserver.argName(i) == "f_WIFI_DEVICENAME") { webserver.arg(webserver.argName(i)).toCharArray(config.WIFI_DEVICENAME, sizeof(config.WIFI_DEVICENAME)); }
      if (webserver.argName(i) == "f_WIFI_RECONDELAY") { config.WIFI_RECONDELAY = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_OTA_HOSTNAME") { webserver.arg(webserver.argName(i)).toCharArray(config.OTA_HOSTNAME, sizeof(config.OTA_HOSTNAME)); }
      if (webserver.argName(i) == "f_OTA_PORT") { config.OTA_PORT = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_NTP_SERVER") { webserver.arg(webserver.argName(i)).toCharArray(config.NTP_SERVER, sizeof(config.NTP_SERVER)); }
      if (webserver.argName(i) == "f_NTP_TZ") { webserver.arg(webserver.argName(i)).toCharArray(config.NTP_TZ, sizeof(config.NTP_TZ)); }
      if (webserver.argName(i) == "f_MQTT_IP") { webserver.arg(webserver.argName(i)).toCharArray(config.MQTT_IP, sizeof(config.MQTT_IP)); }
      if (webserver.argName(i) == "f_MQTT_PORT") { config.MQTT_PORT = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_MQTT_MSGSIZE") { config.MQTT_MSGSIZE = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_MQTT_KEEPALIVE") { config.MQTT_KEEPALIVE1 = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_MQTT_RECONDELAY") { config.MQTT_RECONDELAY = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_MQTT_TOPIC1") { webserver.arg(webserver.argName(i)).toCharArray(config.MQTT_TOPIC1, sizeof(config.MQTT_TOPIC1)); }
      if (webserver.argName(i) == "f_MQTT_TOPIC2") { webserver.arg(webserver.argName(i)).toCharArray(config.MQTT_TOPIC2, sizeof(config.MQTT_TOPIC2)); }
      if (webserver.argName(i) == "f_MQTT_DELIMITER") { webserver.arg(webserver.argName(i)).toCharArray(config.MQTT_DELIMITER, sizeof(config.MQTT_DELIMITER)); }
      if (webserver.argName(i) == "f_MQTT_DEBUG") { config.MQTT_DEBUG = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_MUX") { config.MUX = webserver.arg(webserver.argName(i)).toInt(); }
      //if (webserver.argName(i) == "f_MUX") { webserver.arg(webserver.argName(i)).toCharArray(config.MUX, sizeof(config.MUX)); }
      if (webserver.argName(i) == "f_NUMDISP") { config.NUMDISP = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_DISPWIDTH") { config.DISPWIDTH = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_DISPHEIGHT") { config.DISPHEIGHT = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_STARTDELAY") { config.STARTDELAY = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_UPDSPEED") { config.UPDSPEED = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_SCREENSAVER") { config.SCREENSAVER = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_PRINTBUF") { config.PRINTBUF = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_DPL_ID0") { webserver.arg(webserver.argName(i)).toCharArray(DPL_id[0], sizeof(DPL_id[0])); }
      if (webserver.argName(i) == "f_DPL_ID1") { webserver.arg(webserver.argName(i)).toCharArray(DPL_id[1], sizeof(DPL_id[1])); }
      if (webserver.argName(i) == "f_DPL_ID2") { webserver.arg(webserver.argName(i)).toCharArray(DPL_id[2], sizeof(DPL_id[2])); }
      if (webserver.argName(i) == "f_DPL_ID3") { webserver.arg(webserver.argName(i)).toCharArray(DPL_id[3], sizeof(DPL_id[3])); }
      if (webserver.argName(i) == "f_DPL_ID4") { webserver.arg(webserver.argName(i)).toCharArray(DPL_id[4], sizeof(DPL_id[4])); }
      if (webserver.argName(i) == "f_DPL_ID5") { webserver.arg(webserver.argName(i)).toCharArray(DPL_id[5], sizeof(DPL_id[5])); }
      if (webserver.argName(i) == "f_DPL_ID6") { webserver.arg(webserver.argName(i)).toCharArray(DPL_id[6], sizeof(DPL_id[6])); }
      if (webserver.argName(i) == "f_DPL_ID7") { webserver.arg(webserver.argName(i)).toCharArray(DPL_id[7], sizeof(DPL_id[7])); }
      if (webserver.argName(i) == "f_DPL_STATION00") { webserver.arg(webserver.argName(i)).toCharArray(DPL_station[0], sizeof(DPL_station[0])); }
      if (webserver.argName(i) == "f_DPL_STATION01") { webserver.arg(webserver.argName(i)).toCharArray(DPL_station[1], sizeof(DPL_station[1])); }
      if (webserver.argName(i) == "f_DPL_STATION02") { webserver.arg(webserver.argName(i)).toCharArray(DPL_station[2], sizeof(DPL_station[2])); }
      if (webserver.argName(i) == "f_DPL_STATION03") { webserver.arg(webserver.argName(i)).toCharArray(DPL_station[3], sizeof(DPL_station[3])); }
      if (webserver.argName(i) == "f_DPL_STATION04") { webserver.arg(webserver.argName(i)).toCharArray(DPL_station[4], sizeof(DPL_station[4])); }
      if (webserver.argName(i) == "f_DPL_STATION05") { webserver.arg(webserver.argName(i)).toCharArray(DPL_station[5], sizeof(DPL_station[5])); }
      if (webserver.argName(i) == "f_DPL_STATION06") { webserver.arg(webserver.argName(i)).toCharArray(DPL_station[6], sizeof(DPL_station[6])); }
      if (webserver.argName(i) == "f_DPL_STATION07") { webserver.arg(webserver.argName(i)).toCharArray(DPL_station[7], sizeof(DPL_station[7])); }
      if (webserver.argName(i) == "f_DPL_TRACK0") { webserver.arg(webserver.argName(i)).toCharArray(DPL_track[0], sizeof(DPL_track[0])); }
      if (webserver.argName(i) == "f_DPL_TRACK1") { webserver.arg(webserver.argName(i)).toCharArray(DPL_track[1], sizeof(DPL_track[1])); }
      if (webserver.argName(i) == "f_DPL_TRACK2") { webserver.arg(webserver.argName(i)).toCharArray(DPL_track[2], sizeof(DPL_track[2])); }
      if (webserver.argName(i) == "f_DPL_TRACK3") { webserver.arg(webserver.argName(i)).toCharArray(DPL_track[3], sizeof(DPL_track[3])); }
      if (webserver.argName(i) == "f_DPL_TRACK4") { webserver.arg(webserver.argName(i)).toCharArray(DPL_track[4], sizeof(DPL_track[4])); }
      if (webserver.argName(i) == "f_DPL_TRACK5") { webserver.arg(webserver.argName(i)).toCharArray(DPL_track[5], sizeof(DPL_track[5])); }
      if (webserver.argName(i) == "f_DPL_TRACK6") { webserver.arg(webserver.argName(i)).toCharArray(DPL_track[6], sizeof(DPL_track[6])); }
      if (webserver.argName(i) == "f_DPL_TRACK7") { webserver.arg(webserver.argName(i)).toCharArray(DPL_track[7], sizeof(DPL_track[7])); }
      if (webserver.argName(i) == "f_DPL_FLIP0") { DPL_flip[0] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_DPL_FLIP1") { DPL_flip[1] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_DPL_FLIP2") { DPL_flip[2] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_DPL_FLIP3") { DPL_flip[3] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_DPL_FLIP4") { DPL_flip[4] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_DPL_FLIP5") { DPL_flip[5] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_DPL_FLIP6") { DPL_flip[6] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_DPL_FLIP7") { DPL_flip[7] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_DPL_CONTRAST0") { DPL_contrast[0] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_DPL_CONTRAST1") { DPL_contrast[1] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_DPL_CONTRAST2") { DPL_contrast[2] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_DPL_CONTRAST3") { DPL_contrast[3] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_DPL_CONTRAST4") { DPL_contrast[4] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_DPL_CONTRAST5") { DPL_contrast[5] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_DPL_CONTRAST6") { DPL_contrast[6] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_DPL_CONTRAST7") { DPL_contrast[7] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_DPL_SIDE0") { DPL_side[0] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_DPL_SIDE1") { DPL_side[1] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_DPL_SIDE2") { DPL_side[2] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_DPL_SIDE3") { DPL_side[3] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_DPL_SIDE4") { DPL_side[4] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_DPL_SIDE5") { DPL_side[5] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_DPL_SIDE6") { DPL_side[6] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_DPL_SIDE7") { DPL_side[7] = webserver.arg(webserver.argName(i)).toInt(); }
    }
    saveConfiguration(configfile, config);
  }
}

// TEMPLATE 1 - Fonts, Logos
void loadTpl1()
{
  File htmlTpl = LittleFS.open( "/tpl1.htm", "r" );
  buf1 = htmlTpl.readString();
  htmlTpl.close();
  buf1.replace("%WIFI_DEVICENAME%", String(config.WIFI_DEVICENAME));

  //uint8_t i = 0;
  //String fbuf = "";
  //fbuf = String(&*fontno[i]);
  buf1.replace("%TPL_FONT0%", String(fonts[0]));
  buf1.replace("%TPL_FONT1%", String(fonts[1]));
  buf1.replace("%TPL_FONT2%", String(fonts[2]));
  buf1.replace("%TPL_FONT3%", String(fonts[3]));
  buf1.replace("%TPL_FONT4%", String(fonts[4]));
  buf1.replace("%TPL_FONT5%", String(fonts[5]));
  buf1.replace("%TPL_FONT6%", String(fonts[6]));
  buf1.replace("%TPL_FONT7%", String(fonts[7]));
  buf1.replace("%TPL_FONT8%", String(fonts[8]));
  buf1.replace("%TPL_FONT9%", String(fonts[9]));

  buf1.replace("%TPL_LOGOID0%", String(logoId[0]));
  buf1.replace("%TPL_LOGOID1%", String(logoId[1]));
  buf1.replace("%TPL_LOGOID2%", String(logoId[2]));
  buf1.replace("%TPL_LOGOID3%", String(logoId[3]));
  buf1.replace("%TPL_LOGOID4%", String(logoId[4]));
  buf1.replace("%TPL_LOGOID5%", String(logoId[5]));
  buf1.replace("%TPL_LOGOID6%", String(logoId[6]));
  buf1.replace("%TPL_LOGOID7%", String(logoId[7]));
  buf1.replace("%TPL_LOGOID8%", String(logoId[8]));
  buf1.replace("%TPL_LOGOID9%", String(logoId[9]));
  buf1.replace("%TPL_LOGOID10%", String(logoId[10]));
  buf1.replace("%TPL_LOGOID11%", String(logoId[11]));
  buf1.replace("%TPL_LOGOID12%", String(logoId[12]));
  buf1.replace("%TPL_LOGOID13%", String(logoId[13]));
  buf1.replace("%TPL_LOGOID14%", String(logoId[14]));
  buf1.replace("%TPL_LOGOID15%", String(logoId[15]));
  buf1.replace("%TPL_LOGOID16%", String(logoId[16]));
  buf1.replace("%TPL_LOGOID17%", String(logoId[17]));
  buf1.replace("%TPL_LOGOID18%", String(logoId[18]));
  buf1.replace("%TPL_LOGOID19%", String(logoId[19]));

  buf1.replace("%TPL_LOGO0W%", String(logow[0]));
  buf1.replace("%TPL_LOGO1W%", String(logow[1]));
  buf1.replace("%TPL_LOGO2W%", String(logow[2]));
  buf1.replace("%TPL_LOGO3W%", String(logow[3]));
  buf1.replace("%TPL_LOGO4W%", String(logow[4]));
  buf1.replace("%TPL_LOGO5W%", String(logow[5]));
  buf1.replace("%TPL_LOGO6W%", String(logow[6]));
  buf1.replace("%TPL_LOGO7W%", String(logow[7]));
  buf1.replace("%TPL_LOGO8W%", String(logow[8]));
  buf1.replace("%TPL_LOGO9W%", String(logow[9]));
  buf1.replace("%TPL_LOGO10W%", String(logow[10]));
  buf1.replace("%TPL_LOGO11W%", String(logow[11]));
  buf1.replace("%TPL_LOGO12W%", String(logow[12]));
  buf1.replace("%TPL_LOGO13W%", String(logow[13]));
  buf1.replace("%TPL_LOGO14W%", String(logow[14]));
  buf1.replace("%TPL_LOGO15W%", String(logow[15]));
  buf1.replace("%TPL_LOGO16W%", String(logow[16]));
  buf1.replace("%TPL_LOGO17W%", String(logow[17]));
  buf1.replace("%TPL_LOGO18W%", String(logow[18]));
  buf1.replace("%TPL_LOGO19W%", String(logow[19]));

  buf1.replace("%TPL_LOGO0H%", String(logoh[0]));
  buf1.replace("%TPL_LOGO1H%", String(logoh[1]));
  buf1.replace("%TPL_LOGO2H%", String(logoh[2]));
  buf1.replace("%TPL_LOGO3H%", String(logoh[3]));
  buf1.replace("%TPL_LOGO4H%", String(logoh[4]));
  buf1.replace("%TPL_LOGO5H%", String(logoh[5]));
  buf1.replace("%TPL_LOGO6H%", String(logoh[6]));
  buf1.replace("%TPL_LOGO7H%", String(logoh[7]));
  buf1.replace("%TPL_LOGO8H%", String(logoh[8]));
  buf1.replace("%TPL_LOGO9H%", String(logoh[9]));
  buf1.replace("%TPL_LOGO10H%", String(logoh[10]));
  buf1.replace("%TPL_LOGO11H%", String(logoh[11]));
  buf1.replace("%TPL_LOGO12H%", String(logoh[12]));
  buf1.replace("%TPL_LOGO13H%", String(logoh[13]));
  buf1.replace("%TPL_LOGO14H%", String(logoh[14]));
  buf1.replace("%TPL_LOGO15H%", String(logoh[15]));
  buf1.replace("%TPL_LOGO16H%", String(logoh[16]));
  buf1.replace("%TPL_LOGO17H%", String(logoh[17]));
  buf1.replace("%TPL_LOGO18H%", String(logoh[18]));
  buf1.replace("%TPL_LOGO19H%", String(logoh[19]));

  String lbuf = "";

  buf1.replace("%TPL_LOGO0%", String(lbuf));
  buf1.replace("%TPL_LOGO1%", String(lbuf));
  buf1.replace("%TPL_LOGO2%", String(lbuf));
  buf1.replace("%TPL_LOGO3%", String(lbuf));
  buf1.replace("%TPL_LOGO4%", String(lbuf));
  buf1.replace("%TPL_LOGO5%", String(lbuf));
  buf1.replace("%TPL_LOGO6%", String(lbuf));
  buf1.replace("%TPL_LOGO7%", String(lbuf));
  buf1.replace("%TPL_LOGO8%", String(lbuf));
  buf1.replace("%TPL_LOGO9%", String(lbuf));
  buf1.replace("%TPL_LOGO10%", String(lbuf));
  buf1.replace("%TPL_LOGO11%", String(lbuf));
  buf1.replace("%TPL_LOGO12%", String(lbuf));
  buf1.replace("%TPL_LOGO13%", String(lbuf));
  buf1.replace("%TPL_LOGO14%", String(lbuf));
  buf1.replace("%TPL_LOGO15%", String(lbuf));
  buf1.replace("%TPL_LOGO16%", String(lbuf));
  buf1.replace("%TPL_LOGO17%", String(lbuf));
  buf1.replace("%TPL_LOGO18%", String(lbuf));
  buf1.replace("%TPL_LOGO19%", String(lbuf));

  webserver.setContentLength( buf1.length() );
  webserver.send( 200, "text/html", buf1 );
  buf1 = "";
}

void handleTpl1Select()
{
  if (webserver.args() > 0 ) {
    for ( uint8_t i = 0; i < webserver.args(); i++ ) {
      if (webserver.argName(i) == "f_TPL_NO") { TPL = webserver.arg(webserver.argName(i)).toInt(); }
    }
    loadTpl1();
  }
}

void handleTpl1Submit()
{
  if (webserver.args() > 0 ) {
    for ( uint8_t i = 0; i < webserver.args(); i++ ) {
      if (webserver.argName(i) == "f_TPL_LOGOID0") { webserver.arg(webserver.argName(i)).toCharArray(logoId[0], sizeof(logoId[0])); }
      if (webserver.argName(i) == "f_TPL_LOGOID1") { webserver.arg(webserver.argName(i)).toCharArray(logoId[1], sizeof(logoId[1])); }
      if (webserver.argName(i) == "f_TPL_LOGOID2") { webserver.arg(webserver.argName(i)).toCharArray(logoId[2], sizeof(logoId[2])); }
      if (webserver.argName(i) == "f_TPL_LOGOID3") { webserver.arg(webserver.argName(i)).toCharArray(logoId[3], sizeof(logoId[3])); }
      if (webserver.argName(i) == "f_TPL_LOGOID4") { webserver.arg(webserver.argName(i)).toCharArray(logoId[4], sizeof(logoId[4])); }
      if (webserver.argName(i) == "f_TPL_LOGOID5") { webserver.arg(webserver.argName(i)).toCharArray(logoId[5], sizeof(logoId[5])); }
      if (webserver.argName(i) == "f_TPL_LOGOID6") { webserver.arg(webserver.argName(i)).toCharArray(logoId[6], sizeof(logoId[6])); }
      if (webserver.argName(i) == "f_TPL_LOGOID7") { webserver.arg(webserver.argName(i)).toCharArray(logoId[7], sizeof(logoId[7])); }
      if (webserver.argName(i) == "f_TPL_LOGOID8") { webserver.arg(webserver.argName(i)).toCharArray(logoId[8], sizeof(logoId[8])); }
      if (webserver.argName(i) == "f_TPL_LOGOID9") { webserver.arg(webserver.argName(i)).toCharArray(logoId[9], sizeof(logoId[9])); }
      if (webserver.argName(i) == "f_TPL_LOGOID10") { webserver.arg(webserver.argName(i)).toCharArray(logoId[10], sizeof(logoId[10])); }
      if (webserver.argName(i) == "f_TPL_LOGOID11") { webserver.arg(webserver.argName(i)).toCharArray(logoId[11], sizeof(logoId[11])); }
      if (webserver.argName(i) == "f_TPL_LOGOID12") { webserver.arg(webserver.argName(i)).toCharArray(logoId[12], sizeof(logoId[12])); }
      if (webserver.argName(i) == "f_TPL_LOGOID13") { webserver.arg(webserver.argName(i)).toCharArray(logoId[13], sizeof(logoId[13])); }
      if (webserver.argName(i) == "f_TPL_LOGOID14") { webserver.arg(webserver.argName(i)).toCharArray(logoId[14], sizeof(logoId[14])); }
      if (webserver.argName(i) == "f_TPL_LOGOID15") { webserver.arg(webserver.argName(i)).toCharArray(logoId[15], sizeof(logoId[15])); }
      if (webserver.argName(i) == "f_TPL_LOGOID16") { webserver.arg(webserver.argName(i)).toCharArray(logoId[16], sizeof(logoId[16])); }
      if (webserver.argName(i) == "f_TPL_LOGOID17") { webserver.arg(webserver.argName(i)).toCharArray(logoId[17], sizeof(logoId[17])); }
      if (webserver.argName(i) == "f_TPL_LOGOID18") { webserver.arg(webserver.argName(i)).toCharArray(logoId[18], sizeof(logoId[18])); }
      if (webserver.argName(i) == "f_TPL_LOGOID19") { webserver.arg(webserver.argName(i)).toCharArray(logoId[19], sizeof(logoId[19])); }
      
      if (webserver.argName(i) == "f_TPL_LOGO0W") { logow[0] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO1W") { logow[1] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO2W") { logow[2] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO3W") { logow[3] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO4W") { logow[4] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO5W") { logow[5] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO6W") { logow[6] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO7W") { logow[7] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO8W") { logow[8] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO9W") { logow[9] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO10W") { logow[10] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO11W") { logow[11] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO12W") { logow[12] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO13W") { logow[13] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO14W") { logow[14] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO15W") { logow[15] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO16W") { logow[16] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO17W") { logow[17] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO18W") { logow[18] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO19W") { logow[19] = webserver.arg(webserver.argName(i)).toInt(); }
      
      if (webserver.argName(i) == "f_TPL_LOGO0H") { logoh[0] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO1H") { logoh[1] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO2H") { logoh[2] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO3H") { logoh[3] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO4H") { logoh[4] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO5H") { logoh[5] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO6H") { logoh[6] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO7H") { logoh[7] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO8H") { logoh[8] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO9H") { logoh[9] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO10H") { logoh[10] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO11H") { logoh[11] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO12H") { logoh[12] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO13H") { logoh[13] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO14H") { logoh[14] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO15H") { logoh[15] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO16H") { logoh[16] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO17H") { logoh[17] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO18H") { logoh[18] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_LOGO19H") { logoh[19] = webserver.arg(webserver.argName(i)).toInt(); }
    }
    saveTemplate(templatefile);
  }
}

// TEMPLATE data - Settings of template 0 - 9
void loadTpl2()
{
  File htmlTplh = LittleFS.open( "/tpl2head.htm", "r" );
  buf1 = htmlTplh.readString();
  htmlTplh.close();
  buf1.replace("%WIFI_DEVICENAME%", String(config.WIFI_DEVICENAME));
  buf1.replace("%TPL_ID0%", String(TPL_id[TPL]));
  webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
  webserver.send( 200, "text/html", buf1 );
  buf1 = "";
  
  File htmlTplb = LittleFS.open( "/tpl2body.htm", "r" );
  String buf2 = htmlTplb.readString();
  htmlTplb.close();

  buf2.replace("%TPL_NAME0%", String(TPL_name[TPL]));
  buf2.replace("%TPL_SIDE0%", String(TPL_side[TPL]));
  buf2.replace("%TPL_INV0%", String(TPL_invert[TPL]));
  
  buf2.replace("%TPL_0FONT0%", String(TPL_0font[TPL]));
  buf2.replace("%TPL_0MAXWIDTH0%", String(TPL_0maxwidth[TPL]));
  buf2.replace("%TPL_0FONT20%", String(TPL_0font2[TPL]));
  buf2.replace("%TPL_0DRAWCOLOR0%", String(TPL_0drawcolor[TPL]));
  buf2.replace("%TPL_0FONTMODE0%", String(TPL_0fontmode[TPL]));
  buf2.replace("%TPL_0POSX0%", String(TPL_0posx[TPL]));
  buf2.replace("%TPL_0POSY0%", String(TPL_0posy[TPL]));
  buf2.replace("%TPL_0SCROLL0%", String(TPL_0scroll[TPL]));

  buf2.replace("%TPL_1FONT0%", String(TPL_1font[TPL]));
  buf2.replace("%TPL_1DRAWCOLOR0%", String(TPL_1drawcolor[TPL]));
  buf2.replace("%TPL_1FONTMODE0%", String(TPL_1fontmode[TPL]));
  buf2.replace("%TPL_1POSX0%", String(TPL_1posx[TPL]));
  buf2.replace("%TPL_1POSY0%", String(TPL_1posy[TPL]));

  buf2.replace("%TPL_2FONT0%", String(TPL_2font[TPL]));
  buf2.replace("%TPL_2MAXWIDTH0%", String(TPL_2maxwidth[TPL]));
  buf2.replace("%TPL_2FONT20%", String(TPL_2font2[TPL]));
  buf2.replace("%TPL_2DRAWCOLOR0%", String(TPL_2drawcolor[TPL]));
  buf2.replace("%TPL_2FONTMODE0%", String(TPL_2fontmode[TPL]));
  buf2.replace("%TPL_2POSX0%", String(TPL_2posx[TPL]));
  buf2.replace("%TPL_2POSY0%", String(TPL_2posy[TPL]));
  buf2.replace("%TPL_2SCROLL0%", String(TPL_2scroll[TPL]));

  buf2.replace("%TPL_3FONT0%", String(TPL_3font[TPL]));
  buf2.replace("%TPL_3DRAWCOLOR0%", String(TPL_3drawcolor[TPL]));
  buf2.replace("%TPL_3FONTMODE0%", String(TPL_3fontmode[TPL]));
  buf2.replace("%TPL_3POSX0%", String(TPL_3posx[TPL]));
  buf2.replace("%TPL_3POSY0%", String(TPL_3posy[TPL]));

  buf2.replace("%TPL_4FONT0%", String(TPL_4font[TPL]));
  buf2.replace("%TPL_4DRAWCOLOR0%", String(TPL_4drawcolor[TPL]));
  buf2.replace("%TPL_4FONTMODE0%", String(TPL_4fontmode[TPL]));
  buf2.replace("%TPL_4POSX0%", String(TPL_4posx[TPL]));
  buf2.replace("%TPL_4POSY0%", String(TPL_4posy[TPL]));

  buf2.replace("%TPL_5LOGOX0%", String(TPL_5logox[TPL]));
  buf2.replace("%TPL_5LOGOY0%", String(TPL_5logoy[TPL]));

  buf2.replace("%TPL_6FONT0%", String(TPL_6font[TPL]));
  buf2.replace("%TPL_6MAXWIDTH0%", String(TPL_6maxwidth[TPL]));
  buf2.replace("%TPL_6FONT20%", String(TPL_6font2[TPL]));
  buf2.replace("%TPL_6DRAWCOLOR0%", String(TPL_6drawcolor[TPL]));
  buf2.replace("%TPL_6FONTMODE0%", String(TPL_6fontmode[TPL]));
  buf2.replace("%TPL_6POSX0%", String(TPL_6posx[TPL]));
  buf2.replace("%TPL_6POSY0%", String(TPL_6posy[TPL]));
  buf2.replace("%TPL_6SCROLL0%", String(TPL_6scroll[TPL]));

  buf2.replace("%TPL_6BOXX0%", String(TPL_6boxx[TPL]));
  buf2.replace("%TPL_6BOXY0%", String(TPL_6boxy[TPL]));
  buf2.replace("%TPL_6BOXW0%", String(TPL_6boxw[TPL]));
  buf2.replace("%TPL_6BOXH0%", String(TPL_6boxh[TPL]));

  buf2.replace("%TPL_6DRAWCOLOR20%", String(TPL_6drawcolor2[TPL]));
  buf2.replace("%TPL_6FONTMODE20%", String(TPL_6fontmode2[TPL]));
  buf2.replace("%TPL_6BOX2X0%", String(TPL_6box2x[TPL]));
  buf2.replace("%TPL_6BOX2Y0%", String(TPL_6box2y[TPL]));
  buf2.replace("%TPL_6BOX2W0%", String(TPL_6box2w[TPL]));
  buf2.replace("%TPL_6BOX2H0%", String(TPL_6box2h[TPL]));
  
  webserver.sendContent( buf2 );  
  buf2 = "";
}

void handleTpl2Select()
{
  if (webserver.args() > 0 ) {
    for ( uint8_t i = 0; i < webserver.args(); i++ ) {
      if (webserver.argName(i) == "f_TPL_NO") { TPL = webserver.arg(webserver.argName(i)).toInt(); }
    }
    loadTpl2();
  }
}

void handleTpl2Submit()
{
  if (webserver.args() > 0 ) {
    for ( uint8_t i = 0; i < webserver.args(); i++ ) {
      if (webserver.argName(i) == "f_TPL_NAME0") { webserver.arg(webserver.argName(i)).toCharArray(TPL_name[TPL], sizeof(TPL_name[TPL])); }
      if (webserver.argName(i) == "f_TPL_SIDE0") { TPL_side[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_INV0") { TPL_invert[TPL] = webserver.arg(webserver.argName(i)).toInt(); }

      if (webserver.argName(i) == "f_TPL_0FONT0") { TPL_0font[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_0MAXWIDTH0") { TPL_0maxwidth[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_0FONT20") { TPL_0font2[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_0DRAWCOLOR0") { TPL_0drawcolor[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_0FONTMODE0") { TPL_0fontmode[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_0POSX0") { TPL_0posx[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_0POSY0") { TPL_0posy[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_0SCROLL0") { TPL_0scroll[TPL] = webserver.arg(webserver.argName(i)).toInt(); }

      if (webserver.argName(i) == "f_TPL_1FONT0") { TPL_1font[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_1DRAWCOLOR0") { TPL_1drawcolor[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_1FONTMODE0") { TPL_1fontmode[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_1POSX0") { TPL_1posx[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_1POSY0") { TPL_1posy[TPL] = webserver.arg(webserver.argName(i)).toInt(); }

      if (webserver.argName(i) == "f_TPL_2FONT0") { TPL_2font[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_2MAXWIDTH0") { TPL_2maxwidth[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_2FONT20") { TPL_2font2[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_2DRAWCOLOR0") { TPL_2drawcolor[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_2FONTMODE0") { TPL_2fontmode[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_2POSX0") { TPL_2posx[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_2POSY0") { TPL_2posy[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_2SCROLL0") { TPL_2scroll[TPL] = webserver.arg(webserver.argName(i)).toInt(); }

      if (webserver.argName(i) == "f_TPL_3FONT0") { TPL_3font[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_3DRAWCOLOR0") { TPL_3drawcolor[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_3FONTMODE0") { TPL_3fontmode[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_3POSX0") { TPL_3posx[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_3POSY0") { TPL_3posy[TPL] = webserver.arg(webserver.argName(i)).toInt(); }

      if (webserver.argName(i) == "f_TPL_4FONT0") { TPL_4font[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_4DRAWCOLOR0") { TPL_4drawcolor[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_4FONTMODE0") { TPL_4fontmode[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_4POSX0") { TPL_4posx[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_4POSY0") { TPL_4posy[TPL] = webserver.arg(webserver.argName(i)).toInt(); }

      if (webserver.argName(i) == "f_TPL_5LOGOX0") { TPL_5logox[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_5LOGOY0") { TPL_5logoy[TPL] = webserver.arg(webserver.argName(i)).toInt(); }

      if (webserver.argName(i) == "f_TPL_6FONT0") { TPL_6font[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_6MAXWIDTH0") { TPL_6maxwidth[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_6FONT20") { TPL_6font2[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_6DRAWCOLOR0") { TPL_6drawcolor[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_6FONTMODE0") { TPL_6fontmode[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_6POSX0") { TPL_6posx[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_6POSY0") { TPL_6posy[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_6SCROLL0") { TPL_6scroll[TPL] = webserver.arg(webserver.argName(i)).toInt(); }

      if (webserver.argName(i) == "f_TPL_6BOXX0") { TPL_6boxx[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_6BOXY0") { TPL_6boxy[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_6BOXW0") { TPL_6boxw[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_6BOXH0") { TPL_6boxh[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
  
      if (webserver.argName(i) == "f_TPL_6DRAWCOLOR20") { TPL_6drawcolor2[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_6FONTMODE20") { TPL_6fontmode2[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_6BOX2X0") { TPL_6box2x[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_6BOX2Y0") { TPL_6box2y[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_6BOX2W0") { TPL_6box2w[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
      if (webserver.argName(i) == "f_TPL_6BOX2H0") { TPL_6box2h[TPL] = webserver.arg(webserver.argName(i)).toInt(); }
    }
    switch (TPL) {
    case 0: saveTemplateFile(template00);
            break;
    case 1: saveTemplateFile(template01);
            break;
    case 2: saveTemplateFile(template02);
            break;
    case 3: saveTemplateFile(template03);
            break;
    case 4: saveTemplateFile(template04);
            break;
    case 5: saveTemplateFile(template05);
            break;
    case 6: saveTemplateFile(template06);
            break;
    case 7: saveTemplateFile(template07);
            break;
    case 8: saveTemplateFile(template08);
            break;
    case 9: saveTemplateFile(template09);
    }
  }
}

// TEMPLATE load import template page
void loadTpl2imp()
{
  File htmlTpl = LittleFS.open( "/tpl2imp.htm", "r" );
  buf1 = htmlTpl.readString();
  htmlTpl.close();
  buf1.replace("%WIFI_DEVICENAME%", String(config.WIFI_DEVICENAME));
  webserver.setContentLength( buf1.length() );
  webserver.send( 200, "text/html", buf1 );
  buf1 = "";
}

// TEMPLATE submit import
void handleTpl2impSubmit()
{
  char TPLcontent[1000] = "";
  if (webserver.args() > 0 ) {
    for ( uint8_t i = 0; i < webserver.args(); i++ ) {
      if (webserver.argName(i) == "f_TPL_CONTENT") { webserver.arg(webserver.argName(i)).toCharArray(TPLcontent, sizeof(TPLcontent)); }
    }
  }
  importTemplateFile(TPLcontent);
  //loadTpl2();
}

void downloadFile(){
  if (!LittleFS.begin()) {
    Serial.println("LittleFS failed to mount !\r\n");                   
  }
  else {
    String str = "";
    File f = LittleFS.open(webserver.arg(0), "r");
    if (!f) {
      Serial.println("Can't open LittleFS file !\r\n");         
    }
    else {
      char bufc[1300];
      int siz = f.size();
      while(siz > 0) {
        size_t len = std::min((int)(sizeof(bufc) - 1), siz);
        f.read((uint8_t *)bufc, len);
        bufc[len] = 0;
        str += bufc;
        siz -= sizeof(bufc) - 1;
      }
      f.close();
      webserver.setContentLength( str.length() );
      webserver.send(200, "text/plain", str);
    }
  }
}

// SEC
void loadSec()
{
  File htmlSec = LittleFS.open( "/sec.htm", "r" );
  buf1 = htmlSec.readString();
  htmlSec.close();
  buf1.replace("%WIFI_DEVICENAME%", String(config.WIFI_DEVICENAME));
  buf1.replace("%WIFI_SSID%", String(sec.WIFI_SSID));
  buf1.replace("%MQTT_USER%", String(sec.MQTT_USER));

  webserver.setContentLength( buf1.length() );
  webserver.send( 200, "text/html", buf1 );
  buf1 = "";
}

void handleSecSubmit()
{
  if (webserver.args() > 0 ) {
    for ( uint8_t i = 0; i < webserver.args(); i++ ) {
      if (webserver.argName(i) == "f_WIFI_SSID") { webserver.arg(webserver.argName(i)).toCharArray(sec.WIFI_SSID, sizeof(sec.WIFI_SSID)); }
      if (webserver.argName(i) == "f_WIFI_PW") { webserver.arg(webserver.argName(i)).toCharArray(sec.WIFI_PW, sizeof(sec.WIFI_PW)); }
      if (webserver.argName(i) == "f_OTA_PW") { webserver.arg(webserver.argName(i)).toCharArray(sec.OTA_PW, sizeof(sec.OTA_PW)); }
      if (webserver.argName(i) == "f_OTA_HASH") { webserver.arg(webserver.argName(i)).toCharArray(sec.OTA_HASH, sizeof(sec.OTA_HASH)); }
      if (webserver.argName(i) == "f_MQTT_USER") { webserver.arg(webserver.argName(i)).toCharArray(sec.MQTT_USER, sizeof(sec.MQTT_USER)); }
      if (webserver.argName(i) == "f_MQTT_PW") { webserver.arg(webserver.argName(i)).toCharArray(sec.MQTT_PW, sizeof(sec.MQTT_PW)); }
    }
    saveSec(secfile, sec);
  }
}


// UPDATE
void loadUpdate(String updstatus)
{
  File htmlUpdate = LittleFS.open( "/update.htm", "r" );
  buf1 = htmlUpdate.readString();
  htmlUpdate.close();
  buf1.replace("%VER%", config.VER);
  buf1.replace("%WIFI_DEVICENAME%", String(config.WIFI_DEVICENAME));
  //buf1.replace("%WIFI_IP%", String(WiFi.localIP()));
  buf1.replace("%UPDSTATUS%", "");
  //buf1.replace("%UPDSTATUS%", updstatus);
  webserver.setContentLength( buf1.length() );
  webserver.send( 200, "text/html", buf1 );
  buf1 = "";
}

void loadUpdateStatus(String updstatus)
{
  File htmlUpdate = LittleFS.open( "/update.htm", "r" );
  buf1 = htmlUpdate.readString();
  htmlUpdate.close();
  buf1.replace("%VER%", config.VER);
  buf1.replace("%WIFI_DEVICENAME%", String(config.WIFI_DEVICENAME));
  buf1.replace("%UPDSTATUS%", updstatus);
  webserver.setContentLength( buf1.length() );
  webserver.send( 200, "text/html", buf1 );
  
  yield();
  buf1 = "";
  ESP.restart();
}

#endif
