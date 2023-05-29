// -*- coding: utf-8 -*-

#include <Arduino.h>
#include "RadioLib.h"
#include "renogyrover.hpp"
#include "PeriodicTimer.hpp"
#include "LinearActuator.hpp"
#include "utils.hpp"

#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

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
#define SX1262_FREQ 915.0

#define NODE_ADDRESS 2

#define VBAT 34 // 1  // connected to Vbatt through a 1/2 divider network

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
SX1262 *radio;

void setup()
{
    pinMode(MD10C_PWM_PIN, OUTPUT);
    digitalWrite(MD10C_PWM_PIN, 0);

    // Console
    Serial.begin(115200);
    while (!Serial) delay(10);
    Serial.println("test_gate_hw ");
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

    SX1262 radio = new Module(SX1262_NSS, SX1262_DIO1, SX1262_RST, SX1262_BUSY);
    Serial.print(F("[SX1262] Initializing ... "));
    int state = radio.begin();
    if (state == RADIOLIB_ERR_NONE)
    {
        Serial.println(F("success!"));
    }
    else
    {
        Serial.print(F("failed, code "));
        Serial.println(state);
        while (true);
    }

    // Setup freq, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

    if (radio.setFrequency(SX1262_FREQ) == RADIOLIB_ERR_INVALID_FREQUENCY)
    {
        Serial.println(F("Selected frequency is invalid for this module!"));
        while (true);
    }

    // set bandwidth to 250 kHz
    if (radio.setBandwidth(125.0) == RADIOLIB_ERR_INVALID_BANDWIDTH) {
        Serial.println(F("Selected bandwidth is invalid for this module!"));
        while (true);
    }

    // set spreading factor to 7 (128 chips/symbol)
    if (radio.setSpreadingFactor(7) == RADIOLIB_ERR_INVALID_SPREADING_FACTOR) {
        Serial.println(F("Selected spreading factor is invalid for this module!"));
        while (true);
    }

    // set coding rate to 5
    if (radio.setCodingRate(5) == RADIOLIB_ERR_INVALID_CODING_RATE) {
        Serial.println(F("Selected coding rate is invalid for this module!"));
        while (true);
    }

    // set LoRa sync word to 0xAB
    if (radio.setSyncWord(0xAB) != RADIOLIB_ERR_NONE) {
        Serial.println(F("Unable to set sync word!"));
        while (true);
    }

    // set output power to 10 dBm (accepted range is -17 - 22 dBm)
    if (radio.setOutputPower(10) == RADIOLIB_ERR_INVALID_OUTPUT_POWER) {
        Serial.println(F("Selected output power is invalid for this module!"));
        while (true);
    }

    // set over current protection limit to 80 mA (accepted range is 45 - 240 mA)
    // NOTE: set value to 0 to disable overcurrent protection
    if (radio.setCurrentLimit(80) == RADIOLIB_ERR_INVALID_CURRENT_LIMIT) {
        Serial.println(F("Selected current limit is invalid for this module!"));
        while (true);
    }

    // set LoRa preamble length to 15 symbols (accepted range is 0 - 65535)
    if (radio.setPreambleLength(15) == RADIOLIB_ERR_INVALID_PREAMBLE_LENGTH) {
        Serial.println(F("Selected preamble length is invalid for this module!"));
        while (true);
    }

    if (radio.setOutputPower(13)) {
        Serial.println(F("Error setting output power"));
        while (true);
    }


    // set the function that will be called
    // when packet transmission is finished
    // radio.setDio1Action(setFlag);


    U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R3, SSD1306_RST, SSD1306_SCL, SSD1306_SDA);
    u8g2.begin();
    u8g2.setFont(u8g2_font_6x13_me);
    u8g2.firstPage();
    do {
        u8g2.setCursor(0, 12);
        u8g2.print(F("gate mgr"));

    } while ( u8g2.nextPage() );
}
