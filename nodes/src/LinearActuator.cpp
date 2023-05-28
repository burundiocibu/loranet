// -*- coding: utf-8 -*-
#include "LinearActuator.hpp"
#include "utils.hpp"


uint8_t LinearActuator::pulse_pin = 0;
uint8_t LinearActuator::limit_pin = 0;
long LinearActuator::current_position = 0;
long LinearActuator::target_position = 0;
bool LinearActuator::limit = false;
bool LinearActuator::zero_found = false;
uint32_t LinearActuator::start_time = 0;
MD10C* LinearActuator::motor;


/// @brief constructor for LinearActuator
/// @param _pulse_pin : pin connected to encoder open collector output
/// @param _limit_pin : pin connected to limit switch open collector output
/// @param motor_ptr 
LinearActuator::LinearActuator(uint8_t _pulse_pin, uint8_t _limit_pin, MD10C* motor_ptr)
{
    pulse_pin = _pulse_pin;
    limit_pin = _limit_pin;
    pinMode(pulse_pin, INPUT);
    pinMode(limit_pin, INPUT);
    motor = motor_ptr;
    motor->stop();
    attachInterrupt(digitalPinToInterrupt(pulse_pin), pulse_isr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(limit_pin), limit_isr, RISING);
    goto_position(0);
}

// handle the limit switch being triggered
void LinearActuator::limit_isr()
{
    motor->stop();
    limit = true;
    current_position = 0;
    zero_found = true;
}

// Handle pulses from encoder
void LinearActuator::pulse_isr()
{
    if (motor->get_direction() > 0)
        current_position++;
    else
        current_position--;
    
    // Just update the limit indicator but let the ISR stop the motor
    limit = digitalRead(limit_pin);

    int err = target_position - current_position;
    if (target_position == 0)
    {
        int speed = min(abs(err), 255); // start with the error as the speed
        speed = min(int(dt(start_time)/3), speed); // clamp it to dt/3
        speed = max(speed, 64); // don't go below 64/255
        motor->set_speed(speed);
        return;
    }

    if (err == 0)
    {
        motor->stop();
        return;
    }
    
    motor->set_direction(err);
    int speed = min(abs(err), 255); // start with the error as the speed
    speed = min(int(dt(start_time)/3), speed); // clamp it to dt/3
    speed = max(speed, 64); // don't go below 64/255
    motor->set_speed(speed);
}


long LinearActuator::get_position() { return current_position; }
int LinearActuator::get_speed() { return motor->get_speed(); }
bool LinearActuator::get_limit() { return limit; }


// Fully retracting arm is position 0, limit switch engaged.
// Fully extending arm is max pos, estimated.
void LinearActuator::goto_position(long position)
{
    if (position < 0)
        return;

    if ((position == 0 && !limit) || current_position > position)
        motor->set_direction(-1);
    else if (current_position < position)
        motor->set_direction(1);
    target_position = position;
    // this is just a kick to get it going.
    motor->set_speed(128);
    start_time = millis();
}

String LinearActuator::get_status()
{
    return String(" cp:") + String(current_position) +
        String(", tp:") + String(target_position) +
        String(", ms:") + String(motor->get_speed()) +
        String(", md:") + String(motor->get_direction()) +
        String(", ml:") + String(get_limit());
}

