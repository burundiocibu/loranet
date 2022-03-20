// -*- coding: utf-8 -*-
#include "renogyrover.hpp"

RenogyRover::RenogyRover(Stream &serial)
{
    node.begin(1, serial);
}

uint16_t RenogyRover::read_register(uint16_t reg)
{
    if (node.readHoldingRegisters(reg, 1))
        return 0xffff;
    else
        return node.getResponseBuffer(0x0);
}

String RenogyRover::status_msg()
{
    String msg;
    msg += String("bv:") + String(battery_voltage());
    msg += String(",bp:") + String(battery_percentage());
    msg += String(",bc:") + String(battery_capicity());
    msg += String(",lv:") + String(load_voltage());
    msg += String(",lc:") + String(load_current());
    msg += String(",lo:") + String(load_on());
    msg += String(",sv:") + String(solar_voltage());
    msg += String(",sc:") + String(solar_current());
    msg += String(",cs:") + String(charging_state());
    return msg;
}