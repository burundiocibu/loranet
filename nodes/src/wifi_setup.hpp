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
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(hostname);
    WiFi.begin(ssid, password);

    ArduinoOTA.setHostname(hostname);
    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
    ArduinoOTA.setPassword(ota_password);
}

void ota_handle()
{
    static bool ota_ready = false;

    if (ota_ready)
    {
        ArduinoOTA.handle();
        return;
    }

    static unsigned long last_begin = 0;
    wl_status_t wfs = WiFi.status();
    if (wfs != WL_CONNECTED)
    {
        if (millis() - last_begin > 60000)
        {
            Logger::warning("wifi status: %x, wifi.begin", wfs);
            WiFi.begin(ssid, password);
            last_begin = millis();
        }
        return;
    }

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
