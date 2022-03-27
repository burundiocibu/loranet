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
int txpwr = 23;

int tx_rtt = 0;
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
    //while (!Serial) delay(10);
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


String runtime()
{
    unsigned long ms=millis();
    long sec = ms/1000;
    ms -= sec * 1000;
    long hour = sec / 3600;
    sec -= hour*3600;
    long min = sec/60;
    sec -= min*60;

    char buff[12];
    sprintf(buff, "%ld:%02ld:%02ld.%03ld", hour, min, sec, ms);
    return String(buff);
}


void send_msg(uint8_t dest, String& msg)
{
    Serial.println(runtime() + " Tx to:" + String(dest) + ", msg:" + msg);
    rf95.setTxPower(txpwr);
    unsigned long t0 = millis();
    if (!manager.sendtoWait((uint8_t*)msg.c_str(), msg.length(), dest))
        Serial.println(runtime() + " sendtoWait failed");
    tx_rtt = millis() - t0;
}


void send_update(uint8_t dest)
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
    if (tx_rtt > 0)
        msg += String(",rtt:") + String(tx_rtt);
    send_msg(dest, msg);


    float bv = rover.battery_voltage();
    if (bv < 100)
    {
        msg += String(",bv:") + String(bv);
        msg += String(",bp:") + String(rover.battery_percentage());
        msg += String(",bc:") + String(rover.battery_capicity());
        msg += String(",lv:") + String(rover.load_voltage());
        msg += String(",lc:") + String(rover.load_current());
        msg += String(",lo:") + String(rover.load_on());
        msg += String(",sv:") + String(rover.solar_voltage());
        msg += String(",sc:") + String(rover.solar_current());
        msg += String(",cs:") + String(rover.charging_state());
        send_msg(dest, msg);
    }
}


void set_gate_position(int pos, int dest)
{
    if (pos > 0)
    {
        digitalWrite(GATE_OPEN, 1);
        delay(100);
        digitalWrite(GATE_SAFE, 1);
        delay(10);
        digitalWrite(GATE_OPEN, 0);
        gate_position = 100;
    }
    else
    {
        digitalWrite(GATE_OPEN, 0);
        digitalWrite(GATE_SAFE, 0);
        gate_position = 0;
    }
    send_msg(dest, String("gp:") + String(gate_position));
}


void set_poe(int poe, int dest)
{
    poe_status = poe?1:0;
    digitalWrite(POE_ENABLE, poe_status=poe_status);
    send_msg(dest, String("poe:") + String(poe_status));
}


void set_rover_load(int load, int dest)
{
    rover.load_on(load);
    send_msg(dest, String("ro:") + String(load));
}


unsigned long last_update = 0;

void loop()
{
    uint8_t rf95_buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(rf95_buf);
    uint8_t from;
    
    if (manager.available() && manager.recvfromAck(rf95_buf, &len, &from))
    {
        rf95_buf[len] = '\0';
        String msg((char*)rf95_buf);
        Serial.println(runtime() + " Rx: " + msg);

        if (msg=="GO")      set_gate_position(100, from);
        else if (msg=="GC") set_gate_position(0, from);
        else if (msg=="E1") set_poe(1, from);
        else if (msg=="E0") set_poe(0, from);
        else if (msg=="R1") set_rover_load(1, from);
        else if (msg=="R0") set_rover_load(0, from);
    }

    const long update_rate = 15; // seconds
    long dt = millis()/1000 - last_update;
    if (last_update == 0 || dt > update_rate)
    {
        send_update(0);
        last_update += update_rate;
    }
}
