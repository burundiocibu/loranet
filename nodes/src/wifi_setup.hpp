// -*- coding: utf-8 -*-
#ifndef WIFI_SETUP_HPP
#define WIFI_SETUP_HPP

#include <Arduino.h>
#include "MacroLogger.h"

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "secrets.hpp"

void wifi_setup(const char* hostname)
{
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(hostname);
    ArduinoOTA.setHostname(hostname);
    ArduinoOTA.setPassword(ota_password);
}

void ota_handle()
{
    static bool ota_ready = false;
    static unsigned long last_begin = 0;

    if (ota_ready) {
        ArduinoOTA.handle();
        return;
    }

    if (WiFi.status() != WL_CONNECTED) {
        if ((millis() - last_begin) > 5000) {
            WiFi.begin(ssid, password);
            Logger::info("wifi.begin again");
            last_begin = millis();
            return;
        }
        return;
    }

    Logger::info("WiFi Connected");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Logger::info("ota start");
    })
    .onEnd([]() {
      Logger::info("ota end");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Logger::info("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Logger::info("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Logger::info("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Logger::info("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Logger::info("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Logger::info("Receive Failed");
      else if (error == OTA_END_ERROR) Logger::info("End Failed");
    });

  ArduinoOTA.begin();

  ota_ready = true;
  Logger::info("OTA Ready");
}
#endif
