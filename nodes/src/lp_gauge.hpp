// -*- coding: utf-8 -*-
#include <SPI.h>
#include <RH_RF95.h>
#include <OneWireWrapper.hpp>

// This hardware is an Adafruit LoRa Feather 32u4
#define VBAT 9  // connected to Vbatt through a 1/2 divider network

#define RFM95_RST 4
#define RFM95_INT 7
#define RFM95_CS 8
RH_RF95 rf95(RFM95_CS, RFM95_INT);
#define NODE_ADDRESS 3


//#define DISABLE_SLEEP
#include <DeepSleep.hpp>

#define DS18B20_PWR 6
#define AH49E_OUT 10
#define AH49E_PWR 11
#define DS18B20_D0 12
#define USER_LED 13
#define LIPO_CHARGER_EN PB0 // must be brough high to enable measuring or charging the battery

#define RFM95_RST 4
#define RFM95_INT 7
#define RFM95_CS 8

OneWire ds(DS18B20_D0);


void setup()
{
    Serial.begin(115200);
    Serial.println("driveway_sensor");
    pinMode(VBAT, INPUT);

    pinMode(AH49E_PWR, OUTPUT);
    digitalWrite(AH49E_PWR, LOW);
    pinMode(LIPO_CHARGER_EN, OUTPUT);
    digitalWrite(LIPO_CHARGER_EN, LOW);
    pinMode(DS18B20_PWR, OUTPUT);
    digitalWrite(DS18B20_PWR, LOW);

    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);

    while (!rf95.init())
    {
        Serial.println("LoRa radio init failed");
        while (1);
    }

    if (!rf95.setFrequency(915.0))
    {
        Serial.println("setFrequency failed");
        while (1);
    }

    rf95.setTxPower(20, false);
    rf95.setHeaderFrom(NODE_ADDRESS);
    rf95.setHeaderTo(0);
    rf95.setThisAddress(NODE_ADDRESS);
    rf95.setHeaderFlags(0x1)
}