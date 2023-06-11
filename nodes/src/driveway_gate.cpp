// -*- coding: utf-8 -*-
#include "MacroLogger.h"
#include "driveway_gate.hpp"
#include "utils.hpp"


String status()
{
    // LiIon pack directly connected to processor
    float vbat = analogRead(VBAT) * 1.31 * 3.3 / 1024;

    String msg;
    msg += String("v1:") + String(vbat);
    msg += String(",rssi:") + String(node->get_rssi());
    msg += String(",snr:") + String(node->get_snr());
    msg += String(",ut:") + String (int(uptime()));
    msg += String(",rr:") + String (!digitalRead(REMOTE_RECEIVER));
    String m2  = scc->status();
    if (m2.length())
        msg += String(",") + m2;
    return msg;
}


void loop()
{
    Logger::set_level(Logger::Level::INFO);
    String msg;
    byte sender;
    if (node->get_message(msg, sender))
    {
        Logger::info("Rx:from:%d, %s", sender, msg);
        Logger::trace("On core: %d", xPortGetCoreID());
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
        else if (msg.startsWith("R"))
        {
            scc->load_on(msg.substring(1).toInt());
            delay(200);  // it takes a bit for the scc to actuall do this
            node->send_msg(sender, scc->status());
        }
        else if (msg=="SS")
                node->send_msg(sender, status());
    }

    // yeah, its an active low signal
    uint8_t remote = !digitalRead(REMOTE_RECEIVER);
    if (remote)
        gate->open(90);

    if (gate->update() || remote)
        node->send_msg(0, gate->status());

    static PeriodicTimer update_timer(300 * 1000);
    if (update_timer.time())
        node->send_msg(0, status());

    if (millis() > 2000)
    {
        if (!digitalRead(USER_BUTTON1))
        {
            gate->goto_position(0);
            delay(400); 
        }
    }

    // this takes about 32 ms.
    display->firstPage();
    do {
        display->setCursor(0, 12);
        display->print(F("ap "));
        display->setCursor(14, 12);
        display->print(String(actuator->get_position()));
        display->setCursor(0, 25);
        display->print(F("gp "));
        display->setCursor(14, 25);
        display->print(String((int)gate->get_position()));
    } while ( display->nextPage() );
}