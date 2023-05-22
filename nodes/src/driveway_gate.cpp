// -*- coding: utf-8 -*-

// http://www.airspayce.com/mikem/arduino/RadioHead/
// http://www.airspayce.com/mikem/arduino/RadioHead/classRH__RF95.html
#include <RHReliableDatagram.h>
#include <RH_RF95.h>

#include "renogyrover.hpp"
#include "PeriodicTimer.hpp"
#include "LinearActuator.hpp"
#include "utils.hpp"

// for feather32u4
#define RFM95_RST 4
#define RFM95_INT 7 // This is from the pinout image, not the text
#define RFM95_CS 8
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

#define NODE_ADDRESS 2
RHReliableDatagram manager(rf95, NODE_ADDRESS);

#define FEATHER_VBAT 9  // connected to feather Vbatt through a 1/2 divider network

#define GATE_LOCK 21 // GPIO output to relay
#define DRIVEWAY_RECEIVER A3
#define REMOTE_RECEIVER A2
#define ROVER_VLOAD A11 // (w divider)

#define MD10C_PWM 5
#define MD10C_DIR 10



// 5 to 23 dB on this device
int txpwr = 20;

int tx_rtt = 0;
int gate_position = 0;
int rover_load_enable = 1;

RenogyRover rover(Serial1, 1);


void setup()
{
    // Console
    Serial.begin(115200);
    while (!Serial) delay(10);
    Serial.println("LoRaNet node");

    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);

    pinMode(GATE_LOCK, OUTPUT);
    digitalWrite(GATE_LOCK, LOW);

    pinMode(DRIVEWAY_RECEIVER, INPUT);
    pinMode(REMOTE_RECEIVER, INPUT_PULLUP);

    if (!manager.init())
        Serial.println("manager init failed");

    // modbus
    Serial1.begin(9600);

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

    if (rover.load_on() != rover_load_enable)
        rover.load_on(rover_load_enable);
}


void send_msg(uint8_t dest, String& msg)
{
    Serial.println(runtime() + " Tx to:" + String(dest) + ", msg:" + msg);
    rf95.setTxPower(txpwr);
    unsigned long t0 = millis();
    if (!manager.sendtoWait((uint8_t*)msg.c_str(), msg.length(), dest))
        Serial.println(runtime() + " sendtoWait failed");
    tx_rtt = dt(t0);
}


void send_update(uint8_t dest)
{
    // LiIon pack directly connected to feather
    float feather_vbat = analogRead(FEATHER_VBAT) * 2 * 3.3 / 1024;
    // LiFePo4 battery managed by the renogy mgr
    float gate_vbat = analogRead(ROVER_VLOAD) * 4.29 * 3.3/1024;

    String msg;
    msg += String("gp:") + String(gate_position);
    msg += String(",gvb:") + String(gate_vbat);
    msg += String(",fvb:") + String(feather_vbat);
    msg += String(",rssi:") + String(rf95.lastRssi());
    msg += String(",snr:") + String(rf95.lastSNR());
    msg += String(",txpwr:") + String(txpwr);
    msg += String(",ut:") + String (int(uptime()));
    msg += String(",dr:") + String (digitalRead(DRIVEWAY_RECEIVER));
    msg += String(",rr:") + String (digitalRead(REMOTE_RECEIVER));
    if (tx_rtt > 0)
        msg += String(",rtt:") + String(tx_rtt);
    int rc = rover.battery_voltage() < 100;
    msg += String(",rc:") + String(rc);
    send_msg(dest, msg);
}

void send_position_update(uint8_t dest)
{
    String msg;
    msg += String("gp:") + String(gate_position);
    msg += String(",dr:") + String (digitalRead(DRIVEWAY_RECEIVER));
    msg += String(",rr:") + String (digitalRead(REMOTE_RECEIVER));
    send_msg(dest, msg);
}

void send_rover_update(uint8_t dest)
{
    float bv = rover.battery_voltage();
    if (bv > 100)
        return;

    String msg;
    msg = String("bv:") + String(bv);
    msg += String(",bc:") + String(rover.battery_current());
    msg += String(",bp:") + String(rover.battery_percentage());
    msg += String(",ct:") + String(rover.controller_temperature());
    msg += String(",lp:") + String(rover.load_power());
    msg += String(",lo:") + String(rover.load_on());
    msg += String(",cp:") + String(rover.charging_power());
    msg += String(",cs:") + String(rover.charging_state());
    msg += String(",cf:") + String(rover.controller_fault(), HEX);
    msg += String(",dl:") + String(rover.discharging_limit_voltage());
    send_msg(dest, msg);
}


void set_gate_position(int pos, int dest)
{
    Serial.println(runtime());
}


void set_rover_load(int load, int dest)
{
    rover_load_enable = load;
    rover.load_on(load);
    send_msg(dest, String("lo:") + String(load));
}



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

        if (msg=="GO")       set_gate_position(100, from);
        else if (msg=="GC")  set_gate_position(0, from);
        else if (msg=="R1")  set_rover_load(1, from);
        else if (msg=="R0")  set_rover_load(0, from);
    }

    bool open_gate = digitalRead(DRIVEWAY_RECEIVER) | !digitalRead(REMOTE_RECEIVER);
    if (open_gate)
        send_position_update(0);

    static PeriodicTimer update_timer(30000);
    if (update_timer.time())
        send_update(0);

    static PeriodicTimer rover_update_timer(60000);
    if (rover_update_timer.time())
        send_rover_update(0);
}

