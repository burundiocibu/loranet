// -*- coding: utf-8 -*-
#include "MacroLogger.h"
#include "driveway_gate.hpp"
#include "utils.hpp"

String status(bool include_scc=false)
{
    // LiIon pack directly connected to processor
    // float vbat = analogRead(VBAT) * 1.31 * 3.3 / 1024;

    String msg;
    //    msg += String("v1:") + String(vbat);
    msg = String("rssi:") + String(node->get_rssi());
    msg += String(",snr:") + String(node->get_snr());
    msg += String(",ut:") + String(int(uptime()));
    msg += String(",poe:") + String(digitalRead(POE_ENABLE));
    if (include_scc)
        msg += String(",") + scc->status();
    return msg;
}

void loop()
{
    ArduinoOTA.handle();

    Logger::set_level(Logger::Level::TRACE);
    String msg;
    byte sender;
    if (node->get_message(msg, sender))
    {
        Logger::info("Rx:from:%d, %s", sender, msg);
        if (msg == "GS")
        {
            gate->stop();
            node->send_msg(sender, gate->status());
        }
        else if (msg.startsWith("GP"))
        {
            gate->goto_position(msg.substring(2).toInt());
            node->send_msg(sender, gate->status());
        }
        else if (msg == "G+")
        {
            actuator->goto_position(actuator->get_position() - 10);
            node->send_msg(sender, gate->status());
        }
        else if (msg == "G-")
        {
            actuator->goto_position(actuator->get_position() + 10);
            node->send_msg(sender, gate->status());
        }
        else if (msg.startsWith("AP"))
        {
            actuator->goto_position(msg.substring(2).toInt());
            node->send_msg(sender, gate->status());
        }
        else if (msg == "GSCP")
        {
            gate->set_closed_position(actuator->get_position());
            node->send_msg(sender, gate->status());
        }
        else if (msg.startsWith("R"))
        {
            scc->load_on(msg.substring(1).toInt());
            delay(500); // it takes a bit for the scc to actuall do this
            node->send_msg(sender, scc->status());
        }
        else if (msg.startsWith("POE"))
        {
            digitalWrite(POE_ENABLE, msg.substring(3).toInt());
            node->send_msg(sender, status());
        }
        else if (msg == "SS")
            node->send_msg(sender, status(true) + "," + gate->status());
    }

    // yeah, its an active low signal
    uint8_t remote = !digitalRead(REMOTE_RECEIVER);
    if (remote)
    {
        gate->open(90);
        node->send_msg(0, String("rr:1"));
    }

    if (gate->update())
        node->send_msg(0, gate->status());

    if (millis() > 2000 && !digitalRead(USER_BUTTON1))
    {
        gate->goto_position(0);
        delay(400);
    }

    // this takes about 32 ms.
    display->firstPage();
    do
    {
        display->setCursor(0, 12);
        display->print(F("ap "));
        display->setCursor(14, 12);
        display->print(String(actuator->get_position()));
        display->setCursor(0, 25);
        display->print(F("gp "));
        display->setCursor(14, 25);
        display->print(String((int)gate->get_position()));
    } while (display->nextPage());
}