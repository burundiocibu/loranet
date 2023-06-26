// -*- coding: utf-8 -*-
#include "dev_node.hpp"
#include "utils.hpp"

String status()
{
    // LiIon pack directly connected to processor
    float vbat = analogRead(VBAT) * 1.31 * 3.3 / 1024;

    String msg;
    //    msg += String("v1:") + String(vbat);
    msg = String("rssi:") + String(node->get_rssi());
    msg += String(",snr:") + String(node->get_snr());
    msg += String(",ut:") + String(int(uptime()));
    msg += String(",vb:") + String(vbat,2);
    msg += String("");
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
        if (msg.startsWith("SS"))
            node->send_msg(sender, status());
        else if (msg.startsWith("restart"))
        {
            node->send_msg(sender, "text: restarting");
            esp_restart();
        }
    }

    if (!digitalRead(USER_BUTTON1))
        node->send_msg(0, "text: This is a test");
}