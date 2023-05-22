// -*- coding: utf-8 -*-
#include "LinearActuator.hpp"


long LinearActuator::current_position = 0;
long LinearActuator::target_position = 0;
int LinearActuator::current_speed = 0;
int LinearActuator::target_speed = 0;
uint32_t LinearActuator::update_time = 0;
int LinearActuator::last_speed = 0;

MD10C* LinearActuator::motor;

LinearActuator::LinearActuator(uint8_t encoder_pin, uint8_t limit_pin, MD10C* motor_ptr):
    encoder_pin(encoder_pin), limit_pin(limit_pin)
{
    pinMode(encoder_pin, INPUT);
    pinMode(limit_pin, INPUT);
    motor = motor_ptr;
    attachInterrupt(digitalPinToInterrupt(encoder_pin), encoder_isr, CHANGE);
}

void LinearActuator::encoder_isr()
{
    update_time = micros();
    if (motor->get_dir() > 0)
        current_position++;
    else
        current_position--;

    if (current_position == target_position)
        motor->stop();
}

void LinearActuator::goto_position(long position)
{
    if (current_position > position)
    {
        target_position = position;
        motor->run(50);
    }
    else if (current_position < position)
    {
        target_position = position;
        motor->run(-50);
    }
}

String LinearActuator::get_status()
{
    return String(" pos:") + String(current_position) +
        String(", target_pos:") + String(target_position)+
        String(", speed:") + String(motor->get_speed());
}
