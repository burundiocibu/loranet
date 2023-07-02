// -*- coding: utf-8 -*-
#include <SPI.h>
#include <RH_RF95.h>

// This hardware is an Adafruit LoRa Feather 32u4
#define VBAT 9  // connected to Vbatt through a 1/2 divider network

#define RFM95_RST 4
#define RFM95_INT 7
#define RFM95_CS 8
RH_RF95 rf95(RFM95_CS, RFM95_INT);

#define NODE_ADDRESS 5
#define WAKEUP_PIN 3

void setup()
{
    Serial.begin(115200);

    Serial1.begin(115200);
    Serial1.println("driveway_sensor");
    pinMode(VBAT, INPUT);

    pinMode(WAKEUP_PIN, INPUT);

    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);

    // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
    while (!rf95.init())
    {
        Serial1.println("LoRa radio init failed");
        while (1);
    }

    if (!rf95.setFrequency(915.0))
    {
        Serial1.println("setFrequency failed");
        while (1);
    }

    rf95.setTxPower(20, false);
    rf95.setHeaderFrom(NODE_ADDRESS);
    rf95.setHeaderTo(0);
    rf95.setThisAddress(NODE_ADDRESS);
}
