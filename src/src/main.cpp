/**
 * @file main.cpp
 * @author Petr Vanek (petr@fotoventus.cz)
 * @brief Test application for S1V30120
 * @version 0.1
 * @date 2022-11-27
 *
 * @copyright Copyright (c) 2022 Petr Vanek
 *
 */
#include <Arduino.h>
#include <HardwareSerial.h>
#include <string.h>
#include "S1V30120.h"
#include "cfg_server.h"
#include "ap.h"
#include "configuration.h"
#include "dbl_reset.h"
#include "build_in_led.h"
#include <WiFi.h>
#include "talk_server.h"

// ESP32 - SPI - default pins
#define VSPI_MISO MISO
#define VSPI_MOSI MOSI
#define VSPI_SCLK SCK
#define VSPI_SS SS

// control pins for S1V30120
#define S1V30120_RST  13
#define S1V30120_RDY  34
#define S1V30120_MUTE 12

// TXT
const char *readyTxt = "[:name 3]  system ready"; 
const char *readylbl = "ready"; 
const char *waitlbl = "wait";
const char *errorlbl = "#error S1V30120"; 
const char *limitlbl = "limit error"; 



// globals
SPIClass *vspi = nullptr;
S1V30120 *talker = nullptr;
TalkServer *talsrv = nullptr;
String msg;
ItemFS ifs;
Configuration cfg;
DblReset dbl(&ifs);
BuildInLed  binled(2);


bool loadConfig() {
  bool rc = true;
  cfg.ssid = ifs.readItem(ItemFS::Data::ssid);
  cfg.pass = ifs.readItem(ItemFS::Data::password);
  cfg.ip = ifs.readItem(ItemFS::Data::ip);

  if (cfg.ssid.isEmpty() || cfg.pass.isEmpty()) rc = false;
  return rc;
}


/// @brief Creates an AccessPoint with a web server for configuration. 
void webConfig() {
  
    AP ap;
    CfgServer ws(&ifs);

    if (ap.init()) {
       binled.setState(BuildInLed::State::blink);
       Serial.printf("#Configuration IP: %s\n", ap.getIP().toString().c_str());
       Serial.printf("AP: %s,%s\n", ap.getIP().toString().c_str(), ap.getSSID());
       ws.init(80);
       ws.serveCfgPage();
       while(true)  { 
          binled.update();
          delay(200);
       }

    }
    Serial.printf("#Invalid AP INIT\nSTOP\n");
    binled.setState(BuildInLed::State::error);
    while(true) { binled.update(); }
}

/// @brief terminal input & web input
void setup()
{
  
  // sets serial to slow, compatible with retro computers 
  Serial.begin(9600);
  delay(100);
  Serial.println();
  
  binled.setState(BuildInLed::State::off);

  if (!ifs.init() || !ifs.isRequiredFileExists()) {
     Serial.printf("#Invalid file system, please 'Upload Filesystem Image'! \nSTOP\n");
     binled.setState(BuildInLed::State::error);
    while(true) { binled.update(); }
  }
  
  if (!loadConfig()) {
    Serial.printf("#configuration not found, use web config\n"); 
    webConfig();
  } 

  // configuration - double reset 
  if (dbl.isDblRestActivated()) {
        Serial.println("#double Reset");
        dbl.stop();
        Serial.printf("#filesystem content:\n"); 
        ifs.dumpFiles();
        webConfig();
  } 

  vspi = new SPIClass(VSPI);
  vspi->begin(VSPI_SCLK, VSPI_MISO, VSPI_MOSI, VSPI_SS);
  talker = new S1V30120(vspi, S1V30120_RST, S1V30120_RDY, S1V30120_MUTE);
  
  // decltalk mode
  if (!talker->init(false))
  {
      Serial.println(errorlbl);
      binled.setState(BuildInLed::State::error);
      while(true) { binled.update(); }
  }

  // connect to wifi
  binled.setState(BuildInLed::State::connecting);
  WiFi.begin(cfg.ssid.c_str(), cfg.pass.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }

  Serial.print("#IP:");
  Serial.println(WiFi.localIP());
 
  binled.setState(BuildInLed::State::off);
  binled.update();

  /*
  // detail HW info
  Serial.print("#HW:");
  Serial.println(talker->getHWVersion(), HEX);
  Serial.print("#FW:");
  Serial.println(talker->getFWVersion(), HEX);
  Serial.print("#FEAT:");
  Serial.println(talker->getFWFeatures(), HEX);
  */

  binled.setState(BuildInLed::State::on);
  talker->speak(readyTxt, false, true);
  while (!talker->isFinished()) { binled.update(); }
  binled.setState(BuildInLed::State::off);
  binled.update();

  Serial.println(readylbl);
  msg.clear();

  talsrv = new TalkServer(&ifs, talker, &binled);
  talsrv->init(80);
  talsrv->serveTalkPage();

}

void flushInbound() {
  while(Serial.available() > 0) {
      Serial.read();
    }
}

void serialupdate() {
  // serial  
    while (Serial.available() > 0)
    {
      char ch = Serial.read();
     
      // echo
      Serial.print(ch);
      if (ch == 8 && msg.length() > 0) {
         // backspace
         msg.remove(msg.length()-1);
         continue;
      }

      if (ch == '\r')
      {
        Serial.print(msg.c_str());
        talker->speak(msg, false);
        binled.setState(BuildInLed::State::on);
        while (!talker->isFinished())  { binled.update(); }
        binled.setState(BuildInLed::State::off);
        binled.update();

        msg.clear();
        Serial.println();
        Serial.println(readylbl);
        flushInbound();
      }

      // ignore LF
      if (ch == '\n')
        continue;
      
      // maximum message size overflowed 
      if (msg.length() >= talker->maximumMsgSize) {
        Serial.println(limitlbl);
        msg.clear();
        continue;
      }

      msg += ch;
    }
}

void httpUpdate() {

  if (talker->isRunning()) {
    Serial.println(waitlbl);
    while (!talker->isFinished())  { binled.update(); }
    binled.setState(BuildInLed::State::off);
    binled.update();
    Serial.println(readylbl);
  }
  
}

void loop()
{
  serialupdate();
  httpUpdate();
  dbl.update();
}
