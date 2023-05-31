// -*- coding: utf-8 -*-

#include <Arduino.h>
#include "renogyrover.hpp"
#include "PeriodicTimer.hpp"
#include "LinearActuator.hpp"
#include "LoraNode.hpp"

#include <U8g2lib.h>
#include <Wire.h>
#include <SPI.h>


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

#define NODE_ADDRESS 2

#define VBAT 1  // connected to Vbatt through a 1/2 divider network

#define GATE_LOCK 2
#define DRIVEWAY_RECEIVER 39
#define REMOTE_RECEIVER 38

#define MD10C_PWM_PIN 46
#define MD10C_DIR_PIN 45
#define MD10C_PWM_CHAN 0

#define ENCODER_PULSE_PIN 6
#define ENCODER_LIMIT_PIN 7

#define RENOGY_TXD 47
#define RENOGY_RXD 33

#define USER_BUTTON1 0

MD10C* motor;
LinearActuator* gate;
RenogyRover* scc;
LoraNode* node;
U8G2_SSD1306_128X64_NONAME_1_HW_I2C* display;

void setup()
{
    pinMode(MD10C_PWM_PIN, OUTPUT);
    digitalWrite(MD10C_PWM_PIN, 0);

    // Console
    Serial.begin(115200);
    //while (!Serial) delay(10);
    //while (!Serial.available()) delay(10);
    //Serial.println("Press key to start");
    //char ch = Serial.read();
    Serial.println("driveway_gate");
    pinMode(VBAT, INPUT);

    pinMode(GATE_LOCK, OUTPUT);
    digitalWrite(GATE_LOCK, LOW);

    pinMode(DRIVEWAY_RECEIVER, INPUT);
    pinMode(REMOTE_RECEIVER, INPUT_PULLUP);

    pinMode(USER_BUTTON1, INPUT);

    motor = new MD10C(MD10C_PWM_PIN, MD10C_DIR_PIN, MD10C_PWM_CHAN);
    gate = new LinearActuator(ENCODER_PULSE_PIN, ENCODER_LIMIT_PIN, motor);
    
    Serial1.begin(9600, SERIAL_8N1, RENOGY_TXD, RENOGY_RXD);
    scc = new RenogyRover(Serial1);
    scc->load_on(1);

    node = new LoraNode(SX1262_NSS, SX1262_DIO1, SX1262_RST, SX1262_BUSY, NODE_ADDRESS);

    display = new U8G2_SSD1306_128X64_NONAME_1_HW_I2C(U8G2_R3, SSD1306_RST, SSD1306_SCL, SSD1306_SDA);
    display->begin();
    display->setFont(u8g2_font_6x13_me);
    display->firstPage();
    do {
        display->setCursor(0, 12);
        display->print(F("gate mgr"));

    } while ( display->nextPage() );
}
