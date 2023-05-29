// -*- coding: utf-8 -*-

#include <Arduino.h>
#include "driveway_gate.hpp"


void loop()
{
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


    static PeriodicTimer scc_timer(5000);
    if (scc_timer.time())
        Serial.println(runtime() + " rc:" + String(scc->battery_voltage()) );

}

