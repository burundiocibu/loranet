// -*- coding: utf-8 -*-

#include "LoraNode.hpp"

// 32u4 i/o assignments
#define RFM95_RST 4
#define RFM95_INT 7
#define RFM95_CS 8


// Note that this class should be made a singleton
LoraNode::LoraNode(byte ledPin, uint8_t id) :
    ledPin(ledPin), id(id),
    rf95(RFM95_CS, RFM95_INT),
    manager(rf95, id)
{
    pinMode(ledPin, OUTPUT);
    led_ping(1);
    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);


    Serial.println("LoRaNet node");

    if (!manager.init())
        Serial.println("manager init failed");

    // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
    while (!rf95.init())
    {
        Serial.println("LoRa radio init failed");
        while (1);
    }
    Serial.println("LoRa radio init OK!");

    if (!rf95.setFrequency(915.0))
    {
        Serial.println("setFrequency failed");
        while (1);
    }
}

void LoraNode::led_ping(unsigned ms)
{
    digitalWrite(ledPin, HIGH);
    delay(ms);
    digitalWrite(ledPin, LOW);
}

long LoraNode::dt(unsigned long start_time)
{
    unsigned long now = millis();
    if (now < start_time)
        return now - start_time + 0x10000;
    else
        return now - start_time;
}

void LoraNode::send_msg(uint8_t dest, String& msg)
{
    Serial.println("Tx to:" + String(dest) + ", msg:" + msg);
    rf95.setTxPower(txpwr);
    unsigned long t0 = millis();
    if (!manager.sendtoWait((uint8_t*)msg.c_str(), msg.length(), dest))
        Serial.println(" sendtoWait failed");
    tx_rtt = dt(t0);
}
