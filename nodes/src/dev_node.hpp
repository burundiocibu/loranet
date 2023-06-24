// -*- coding: utf-8 -*-
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <SPI.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <AsyncTCP.h>
#endif

#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

// This seems to not like being included before the AsyncTCP stuff
#include <MacroLogger.h>

#include "PeriodicTimer.hpp"
#include "LoraNode.hpp"

// This hardware is a Heltec wifi-lora esp32 v3 module

// SSD1306 OLED display
#define SSD1306_SCL 18
#define SSD1306_SDA 17
#define SSD1306_RST 21

// SX1262 LoRa radio
#define SX1262_NSS 8
#define SX1262_RST 12
#define SX1262_BUSY 13
#define SX1262_DIO1 14

#define NODE_ADDRESS 4

#define VBAT 1  // connected to Vbatt through a 1/2 divider network

#define USER_BUTTON1 0

LoraNode* node;
U8G2_SSD1306_128X64_NONAME_1_HW_I2C* display;


const char* ssid = "groot3-iot";
const char* password = "tmy@tmf2XBP3fdu8kxw";

AsyncWebServer server(80);


void setup()
{
    // Console
    Serial.begin(115200);
    Logger::set_level(Logger::Level::INFO);
    Logger::info("dev_node");
    pinMode(VBAT, INPUT);

    pinMode(USER_BUTTON1, INPUT);

    node = new LoraNode(SX1262_NSS, SX1262_DIO1, SX1262_RST, SX1262_BUSY, NODE_ADDRESS);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("");

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "loranet/dev_node");
    });

    AsyncElegantOTA.begin(&server);    // Start AsyncElegantOTA
    server.begin();
    Serial.println("HTTP server started");

    display = new U8G2_SSD1306_128X64_NONAME_1_HW_I2C(U8G2_R3, SSD1306_RST, SSD1306_SCL, SSD1306_SDA);
    display->begin();
    display->setFont(u8g2_font_6x13_me);
    display->firstPage();
    do {
        display->setCursor(0, 12);
        display->print(F("dev_node"));
    } while ( display->nextPage() );



}
