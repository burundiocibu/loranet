// -*- coding: utf-8 -*-
#include "MacroLogger.h"
#include "driveway_gate.hpp"
#include "utils.hpp"


void send_msg(uint8_t dest, String& msg)
{
    if (node->send_msg(dest, msg))
        Logger::trace("to:%d, %s", dest, msg.c_str());
}


void send_gate_status(uint8_t dest)
{
    // LiIon pack directly connected to feather
    float vbat = analogRead(VBAT) * 1.31 * 3.3 / 1024;

    String msg;
    msg += String("v1:") + String(vbat);
    msg += String(",rssi:") + String(node->get_rssi());
    msg += String(",snr:") + String(node->get_snr());
    msg += String(",ut:") + String (int(uptime()));
    msg += String(",rr:") + String (!digitalRead(REMOTE_RECEIVER));
    float bv = scc->battery_voltage();
    if (bv < 100)
    {
        msg += String(",bv:") + String(bv);
        msg += String(",bc:") + String(scc->battery_current());
        msg += String(",bp:") + String(scc->battery_percentage());
        msg += String(",ct:") + String(scc->controller_temperature());
        msg += String(",lp:") + String(scc->load_power());
        msg += String(",lo:") + String(scc->load_on());
        msg += String(",cp:") + String(scc->charging_power());
        msg += String(",cs:") + String(scc->charging_state());
        msg += String(",cf:") + String(scc->controller_fault(), HEX);
        msg += String(",dl:") + String(scc->discharging_limit_voltage());
    }
    send_msg(dest, msg);
}


void send_position_update(uint8_t dest)
{
    String msg;
    msg += String("gp:") + String(long(gate->get_position()));
    msg += String(",ap:") + String(actuator->get_position());
    msg += String(",rr:") + String (!digitalRead(REMOTE_RECEIVER));
    send_msg(dest, msg);
}


void loop()
{
    String msg;
    byte sender;
    if (node->get_message(msg, sender))
    {
        Logger::info("Rx:%s", msg);
        if (msg=="GS")
            gate->stop();
        else if (msg.startsWith("GP"))
            gate->goto_position(msg.substring(2).toInt());
        else if (msg=="G+")
            actuator->goto_position(actuator->get_position()-10);
        else if (msg=="G-")
            actuator->goto_position(actuator->get_position()+10);
        else if (msg.startsWith("AP"))
            actuator->goto_position(msg.substring(2).toInt());
        else if (msg=="GSCP")
            gate->set_closed_position(actuator->get_position());
        else if (msg=="R1")
            scc->load_on(1);
        else if (msg=="R0")
            scc->load_on(0);
        else if (msg=="SS")
            send_gate_status(0);
        else if (msg.startsWith("LOCK"))
            digitalWrite(GATE_LOCK, msg.substring(4).toInt());
    }

    // yeah, its an active low signal
    uint8_t remote = !digitalRead(REMOTE_RECEIVER);
    if (remote)
        gate->open(90);

    if (gate->update() || remote)
        send_position_update(0);

    static PeriodicTimer update_timer(60000);
    if (update_timer.time())
        send_gate_status(0);

    if (millis() > 2000)
    {
        if (!digitalRead(USER_BUTTON1))
        {
            delay(400);
        }
    }


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