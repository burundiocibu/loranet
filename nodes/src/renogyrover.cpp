// -*- coding: utf-8 -*-
#include <MacroLogger.h>
#include "renogyrover.hpp"

RenogyRover::RenogyRover(Stream &serial, uint8_t load)
{
    node.begin(1, serial);
}

uint16_t RenogyRover::read_register(uint16_t reg)
{
    if (node.readHoldingRegisters(reg, 1))
        return 0xffff;
    else {
        uint16_t resp = node.getResponseBuffer(0x0);
        //Serial.println("Register " + String(reg, HEX) + ":" + String(resp));
        return resp;
    }
}


String RenogyRover::status()
{
    String msg;
    float bv = battery_voltage();
    if (bv < 100)
    {
        msg = String("bv:") + String(bv);
        msg += String(",bc:") + String(battery_current());
        msg += String(",bp:") + String(battery_percentage());
        msg += String(",ct:") + String(controller_temperature());
        msg += String(",lp:") + String(load_power());
        msg += String(",lo:") + String(load_on());
        msg += String(",cp:") + String(charging_power());
        msg += String(",cs:") + String(charging_state());
        msg += String(",cf:") + String(controller_fault(), HEX);
        msg += String(",dl:") + String(discharging_limit_voltage());
    }
    else
        msg = String("cf:ce");

    return msg;
}