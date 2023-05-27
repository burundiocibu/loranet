// -*- coding: utf-8 -*-
#include "LinearActuator.hpp"


uint8_t encoder_pin = 0;
uint8_t limit_pin = 0;
long current_position = 0;
long target_position = 0;
bool limit = false;
int target_speed = 0;
uint32_t update_time = 0;

MD10C* motor;


ISR (PCINT0_vect)
{
    if (!limit)
    {
        motor->stop();
        limit = true;
        current_position = 0;
    }
}


void encoder_isr()
{
    update_time = micros();
    if (motor->get_dir() > 0)
        current_position++;
    else
        current_position--;
    
    limit = digitalRead(limit_pin);

    long err = target_position - current_position;
    if (err == 0)
        motor->stop();
}


LinearActuator::LinearActuator(uint8_t _encoder_pin, uint8_t _limit_pin, MD10C* motor_ptr)
{
    encoder_pin = _encoder_pin;
    limit_pin = _limit_pin;
    pinMode(encoder_pin, INPUT);
    pinMode(limit_pin, INPUT);
    motor = motor_ptr;
    motor->stop();
    attachInterrupt(digitalPinToInterrupt(encoder_pin), encoder_isr, FALLING);

    // this is for PB7
    PCICR = _BV(PCIE0);
    PCMSK0 = _BV(PCINT7);

    //attachInterrupt(PCIE0, limit_isr, RISING);
    reset();
}


// drive arm to limit switch and set pos to zero
void LinearActuator::reset()
{
    current_position = 9999999;
    goto_position(0);
}


bool LinearActuator::get_limit_sw()
{
    limit = digitalRead(limit_pin);
    return limit;
}



long LinearActuator::get_position() { return current_position; }
int LinearActuator::get_speed() { return motor->get_speed(); }


// Fully retracting arm is position 0, limit switch engaged.
// Fully extending arm is max pos, estimated.
void LinearActuator::goto_position(long position)
{
    const long speed=100;
    if (current_position > position)
    {
        target_position = position;
        motor->run(-speed);
    }
    else if (current_position < position)
    {
        target_position = position;
        motor->run(+speed);
    }
}

String LinearActuator::get_status()
{
    return String(" cp:") + String(current_position) +
        String(", tp:") + String(target_position) +
        String(", ms:") + String(motor->get_speed()) +
        String(", ml:") + String(get_limit_sw());
}

