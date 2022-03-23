// -*- coding: utf-8 -*-
#include "renogyrover.hpp"

#include <SPI.h>

// http://www.airspayce.com/mikem/arduino/RadioHead/
// http://www.airspayce.com/mikem/arduino/RadioHead/classRH__RF95.html
#include <RHReliableDatagram.h>
#include <RH_RF95.h>


// for feather32u4 
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7 // This is from the pinout image, not the text

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

#define NODE_ADDRESS 2
RHReliableDatagram manager(rf95, NODE_ADDRESS);

#define FEATHER_VBAT 9  // connected to feather Vbatt through a 1/2 divider network

#define GATE_SAFE 21 // GPIO output to relay
#define GATE_OPEN 22 // GPIO output to relay
#define POE_ENABLE 23 // GPIO output to relay
#define GATE_ENABLE 20  // gate controler on/off

#define GATE_VBAT A11 // (w divider)

// 5 to 23 dB on this device
int txpwr = 5;

int gate_position = 0;
int poe_status = 1;
int gate_enable_status = 1;

RenogyRover rover(Serial1);

void setup() 
{
    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);

    pinMode(GATE_OPEN, OUTPUT);
    digitalWrite(GATE_OPEN, LOW);
    pinMode(GATE_SAFE, OUTPUT);
    digitalWrite(GATE_SAFE, LOW);

    pinMode(GATE_ENABLE, OUTPUT);
    digitalWrite(GATE_ENABLE, gate_enable_status);

    pinMode(POE_ENABLE, OUTPUT);
    digitalWrite(POE_ENABLE, poe_status);

    // modbus
    Serial1.begin(9600);

    // Console
    Serial.begin(115200);
    while (!Serial)
        delay(10);
    Serial.println("LoRaNet node");

    if (!manager.init())
        Serial.println("manager init failed");

    // manual reset the radio
    if (false)
    {
        digitalWrite(RFM95_RST, LOW);
        delay(10);
        digitalWrite(RFM95_RST, HIGH);
        delay(10);
    }

    // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
    while (!rf95.init())
    {
        Serial.println("LoRa radio init failed");
        while (1);
    }
    Serial.println("LoRa radio init OK!");

    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
    if (!rf95.setFrequency(RF95_FREQ))
    {
        Serial.println("setFrequency failed");
        while (1);
    }
    Serial.print("Set Freq to: ");
    Serial.println(RF95_FREQ);
  
    rf95.setTxPower(txpwr, false);
}


void send_msg(uint8_t from, String& msg)
{
    Serial.println(msg);
    rf95.setTxPower(txpwr);
    if (!manager.sendtoWait((uint8_t*)msg.c_str(), msg.length(), from))
        Serial.println("sendtoWait failed");
}


String status_msg()
{
    float feather_vbat = analogRead(FEATHER_VBAT) * 2 * 3.3 / 1024;
    float gate_vbat = analogRead(GATE_VBAT) * 4.29 * 3.3/1024;

    String msg;
    msg += String("gp:") + String(gate_position);
    msg += String(",gvb:") + String(gate_vbat);
    msg += String(",fvb:") + String(feather_vbat);
    msg += String(",rssi:") + String(rf95.lastRssi());
    msg += String(",snr:") + String(rf95.lastSNR());
    msg += String(",txpwr:") + String(txpwr);
    msg += String(",ut:") + String (millis()/1000);
    msg += String(",poe:") + String(poe_status);
    msg += String(",ge:") + String(gate_enable_status);
    msg += String(",bv:") + String(rover.battery_voltage());
    msg += String(",bp:") + String(rover.battery_percentage());
    msg += String(",bc:") + String(rover.battery_capicity());
    msg += String(",lv:") + String(rover.load_voltage());
    msg += String(",lc:") + String(rover.load_current());
    msg += String(",lo:") + String(rover.load_on());
    msg += String(",sv:") + String(rover.solar_voltage());
    msg += String(",sc:") + String(rover.solar_current());
    msg += String(",cs:") + String(rover.charging_state());
    return msg;
}


void open_gate()
{
    digitalWrite(GATE_OPEN, 1);
    delay(100);
    digitalWrite(GATE_SAFE, 1);
    delay(10);
    digitalWrite(GATE_OPEN, 0);
    gate_position = 100;
}


void close_gate()
{
    digitalWrite(GATE_OPEN, 0);
    digitalWrite(GATE_SAFE, 0);
    gate_position = 0;
}


void loop()
{
    if (!manager.available())
    {
        delay(10);
        return;
    }

    uint8_t rf95_buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(rf95_buf);
    uint8_t from;

    if (!manager.recvfromAck(rf95_buf, &len, &from))
    {
        Serial.println("recvfromAck error");
        return;
    }

    rf95_buf[len] = '\0';
    String msg((char*)rf95_buf);
    Serial.print(msg);
    if (msg=="GO")
        open_gate();
    else if (msg=="GC")
        close_gate();
    else if (msg=="E1")
        digitalWrite(POE_ENABLE, poe_status=1);
    else if (msg=="E0")
        digitalWrite(POE_ENABLE, poe_status=0);
    else if (msg=="R1")
        rover.load_on(1);
    else if (msg=="R0")
        rover.load_on(0);
    msg = status_msg();
    send_msg(from, msg);
}
