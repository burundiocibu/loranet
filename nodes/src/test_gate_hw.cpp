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

    display->firstPage();
    do {
        display->setCursor(0, 12);
        display->print(F("gate mgr"));
        display->setCursor(0, 24);
        display->print(F("gp:"));
        display->setCursor(14, 24);
        display->print(String(gate->get_position()));
    } while ( display->nextPage() );
}

