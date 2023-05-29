// -*- coding: utf-8 -*-

#include "driveway_gate.hpp"

int tx_rtt = 0;

void send_msg(uint8_t dest, String& msg)
{
    /*
    Each message sent and received by a RadioHead driver includes 4 headers:
TO The node address that the message is being sent to (broadcast RH_BROADCAST_ADDRESS (255) is permitted)
FROM The node address of the sending node
ID A message ID, distinct (over short time scales) for each message sent by a particilar node
FLAGS A bitmask of flags. The most significant 4 bits are reserved for use by RadioHead. The least significant 4 bits are reserved for applications.

*/
    Serial.println(runtime() + " Tx to:" + String(dest) + ", msg:" + msg);
    unsigned long t0 = millis();
//    if (!manager.sendtoWait((uint8_t*)msg.c_str(), msg.length(), dest))
//        Serial.println(runtime() + " sendtoWait failed");
    tx_rtt = dt(t0);
}


void send_update(uint8_t dest)
{
    // LiIon pack directly connected to feather
    float vbat = analogRead(VBAT) * 2 * 3.3 / 1024;

    String msg;
    msg += String("gp:") + String(gate->get_position());
    msg += String(",vb:") + String(vbat);
//    msg += String(",rssi:") + String(rf95.lastRssi());
//    msg += String(",snr:") + String(rf95.lastSNR());
//    msg += String(",txpwr:") + String(txpwr);
    msg += String(",ut:") + String (int(uptime()));
    msg += String(",dr:") + String (digitalRead(DRIVEWAY_RECEIVER));
    msg += String(",rr:") + String (digitalRead(REMOTE_RECEIVER));
    if (tx_rtt > 0)
        msg += String(",rtt:") + String(tx_rtt);
    int rc = scc->battery_voltage() < 100;
    msg += String(",rc:") + String(rc);
    send_msg(dest, msg);
}

void send_position_update(uint8_t dest)
{
    String msg;
    msg += String("gp:") + String(gate->get_position());
    msg += String(",dr:") + String (digitalRead(DRIVEWAY_RECEIVER));
    msg += String(",rr:") + String (digitalRead(REMOTE_RECEIVER));
    send_msg(dest, msg);
}

void send_scc_update(uint8_t dest)
{
    float bv = scc->battery_voltage();
    if (bv > 100)
        return;

    String msg;
    msg = String("bv:") + String(bv);
    msg += String(",bc:") + String(scc->battery_current());
    msg += String(",bp:") + String(scc->battery_percentage());
    msg += String(",ct:") + String(scc->controller_temperature());
    msg += String(",lp:") + String(scc->load_power());
    msg += String(",lo:") + String(scc->load_on());
    msg += String(",cp:") + String(scc->charging_power());
    msg += String(",cs:") + String(scc->charging_state());
    msg += String(",cf:") + String(scc->controller_fault(), HEX);
    msg += String(",dl:") + String(scc->discharging_limit_voltage());
    send_msg(dest, msg);
}


void loop()
{
    static uint8_t from = 0;
    if (receivedFlag)
    {
        receivedFlag = false;
        String msg;
        int state = radio->readData(msg);
        if (state == RADIOLIB_ERR_NONE)
        {
            Serial.println(runtime() + " Rx: " + msg);
            if (msg=="GO")       gate->goto_position(800);
            else if (msg=="GC")  gate->goto_position(0);
            else if (msg=="R1")  scc->load_on(1);
            else if (msg=="R0")  scc->load_on(0);
        }
        else
            Serial.println(runtime() + " LoRa rx error ");
    }

    bool open_gate = digitalRead(DRIVEWAY_RECEIVER) | !digitalRead(REMOTE_RECEIVER);
    if (open_gate)
        send_position_update(0);

    static PeriodicTimer update_timer(30000);
    if (update_timer.time())
        send_update(0);

    static PeriodicTimer ssc_update_timer(30000);
    if (ssc_update_timer.time())
        send_scc_update(0);

    static PeriodicTimer gate_timer(1000);
    if (gate_timer.time())
        Serial.println(runtime() + gate->get_status());

    if (millis() > 2000)
    {
        if (!digitalRead(USER_BUTTON1))
        {
            gate->goto_position(0);
            Serial.println(runtime() + " goto 0");
            delay(400);
        }
    }

    if (gate->save_position())
        Serial.println(runtime() + " position saved");

    display->firstPage();
    do {
        display->setCursor(0, 11);
        display->print(F("gate mgr"));
        display->setCursor(0, 24);
        display->print(F("gp "));
        display->setCursor(14, 24);
        display->print(String(gate->get_position()));
    } while ( display->nextPage() );
}