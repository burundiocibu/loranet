// -*- coding: utf-8 -*-
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
