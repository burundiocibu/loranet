// -*- coding: utf-8 -*-
#include "driveway_gate.hpp"
#include "utils.hpp"


void send_msg(uint8_t dest, String& msg)
{
    if (node->send_msg(dest, msg))
        Serial.println(runtime() + " Tx to:" + String(dest) + ", msg:" + msg);
    else
        Serial.println(runtime() + " Transmitter busy");
}


void send_gate_status(uint8_t dest)
{
    // LiIon pack directly connected to feather
    float vbat = analogRead(VBAT) * 2 * 3.3 / 1024;

    String msg;
    msg += String("gp:") + String(long(gate->get_position()));
    msg += String(",ap:") + String(actuator->get_position());
    msg += String(",vb:") + String(vbat);
    msg += String(",rssi:") + String(node->get_rssi());
    msg += String(",snr:") + String(node->get_snr());
    msg += String(",ut:") + String (int(uptime()));
    msg += String(",dr:") + String (digitalRead(DRIVEWAY_RECEIVER));
    msg += String(",rr:") + String (digitalRead(REMOTE_RECEIVER));
    int rc = scc->battery_voltage() < 100;
    msg += String(",rc:") + String(rc);
    send_msg(dest, msg);
}


void send_position_update(uint8_t dest)
{
    String msg;
    msg += String("gp:") + String(long(gate->get_position()));
    msg += String(",dr:") + String (digitalRead(DRIVEWAY_RECEIVER));
    msg += String(",rr:") + String (digitalRead(REMOTE_RECEIVER));
    send_msg(dest, msg);
}

void send_scc_status(uint8_t dest)
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
    static PeriodicTimer gate_timer(10000000);

    String msg;
    byte sender;
    if (node->get_message(msg, sender))
    {
        Serial.println(runtime() + " Rx: " + msg);
        if (msg=="GO")
        {
            gate->goto_position(100);
            gate_timer.set_interval(200);
        }
        else if (msg=="GC")
        {
            gate->goto_position(0);
            gate_timer.set_interval(200);
        }
        else if (msg=="G+")
            actuator->goto_position(actuator->get_position()-10);
        else if (msg=="G-")
            actuator->goto_position(actuator->get_position()+10);
        else if (msg=="SGC")
            gate->set_closed_position(actuator->get_position());
        else if (msg=="R1")
            scc->load_on(1);
        else if (msg=="R0")
            scc->load_on(0);
        else if (msg=="GS")
            send_gate_status(0);
        else if (msg=="SS")
            send_scc_status(0);
    }

    bool open_gate = digitalRead(DRIVEWAY_RECEIVER) | !digitalRead(REMOTE_RECEIVER);
    if (open_gate)
        send_position_update(0);

    static PeriodicTimer update_timer(60000);
    if (update_timer.time())
        send_gate_status(0);

    static PeriodicTimer ssc_update_timer(60000);
    if (ssc_update_timer.time())
        send_scc_status(0);

    if (gate->get_speed() == 0)
        gate_timer.set_interval(9999999);
    if (gate_timer.time())
        send_position_update(0);

    if (millis() > 2000)
    {
        if (!digitalRead(USER_BUTTON1))
        {
            actuator->goto_position(0);
            Serial.println(runtime() + " retract arm");
            delay(400);
        }
    }

    if (actuator->save_position())
        Serial.println(runtime() + " position saved");


    // this takes about 32 ms.
    display->firstPage();
    do {
        display->setCursor(0, 11);
        display->print(F("gate mgr"));
        display->setCursor(0, 24);
        display->print(F("ap "));
        display->setCursor(14, 24);
        display->print(String(actuator->get_position()));
        display->setCursor(0, 37);
        display->print(F("gp "));
        display->setCursor(14, 37);
        display->print(String((int)gate->get_position()));
    } while ( display->nextPage() );
}