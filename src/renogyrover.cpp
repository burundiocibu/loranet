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