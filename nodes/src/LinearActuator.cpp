// -*- coding: utf-8 -*-
#include <MacroLogger.h>
#include "Preferences.h"
#include "LinearActuator.hpp"
#include "utils.hpp"


uint8_t LinearActuator::pulse_pin = 0;
uint8_t LinearActuator::limit_pin = 0;
volatile long LinearActuator::noise_counter = 0;
volatile long LinearActuator::current_position = 0;
volatile long LinearActuator::target_position = 0;
volatile bool LinearActuator::limit = false;
volatile uint32_t LinearActuator::start_time = 0;
MD10C* LinearActuator::motor;
volatile bool LinearActuator::dirty_position = false;
volatile bool LinearActuator::stopped = true;

Preferences preferences;

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

    limit = digitalRead(limit_pin);

    int flag_value = -1;
    preferences.begin("LinearActuator");
    current_position = preferences.getInt("position", flag_value);
    preferences.end();
    target_position = current_position;

    if (current_position == flag_value)
        goto_position(0);

    attachInterrupt(digitalPinToInterrupt(pulse_pin), pulse_isr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(limit_pin), limit_isr, RISING);
}


// handle the limit switch being triggered
void LinearActuator::limit_isr()
{
    motor->stop();
    stopped = true;
    limit = true;
    current_position = 0;
}


// Handle pulses from encoder
void LinearActuator::pulse_isr()
{
    static volatile uint32_t last_edge=0;
    uint32_t volatile this_edge = micros();
    int tsle = this_edge - last_edge;
    if (tsle < 0)
        tsle += 0x10000;
    last_edge = this_edge;

    // debounce; ignre pulese less than 4ms
    if (tsle < 4000)
        return;

    if (motor->get_direction() > 0)
        current_position++;
    else
        current_position--;
    dirty_position = true;
    
    limit = digitalRead(limit_pin);

    int err = target_position - current_position;
    if (err == 0)
        motor->stop();

    if (stopped)
        return;
        
    if (target_position > 0)
        motor->set_direction(err);
    else
        motor->set_direction(-1);
    int speed = 0;
    if (current_position < 0)
        speed = 64;
    else
    {   
        err = abs(err);
        speed = min(abs(2*err), 255); // start with the error as the speed
        speed = min(int(dt(start_time)/4), speed); // clamp it to dt
        speed = max(speed, 64); // don't go below 64/255
    }
    motor->set_speed(speed);
}


long LinearActuator::get_position() { return current_position; }
int LinearActuator::get_speed() { return motor->get_speed(); }
bool LinearActuator::get_limit() { return limit = digitalRead(limit_pin); }
bool LinearActuator::get_stopped() { return stopped; }


void LinearActuator::stop()
{
    motor->stop();
    stopped = true;
}


// Fully retracting arm is position 0, limit switch engaged.
// Fully extending arm is max pos, estimated.
void LinearActuator::goto_position(long position)
{
    if (position > 2250)
        return;
    if (position <= 0)
    {
        motor->set_direction(-1);
        if (limit)
        {
            current_position = 0;
            target_position = 0;
            save_position();
            return;
        }
        // if there have been phantom pulsed to indicate its past limit
        // bump the position so the isr doesn't reverse it immedietly
        if (current_position < 0)
            current_position = 50;
        position = 0;
    }
    else if (current_position > position)
        motor->set_direction(-1);
    else if (current_position < position)
        motor->set_direction(1);
    target_position = position;
    // this is just a kick to get it going.
    stopped = false;
    motor->set_speed(64);
    start_time = millis();
}


void LinearActuator::save_position()
{
    if (!dirty_position || motor->get_speed())
        return;
    preferences.begin("LinearActuator");
    preferences.putInt("position", current_position);
    preferences.end();
    dirty_position = false;
    Logger::info("position saved %d", current_position);
}


void LinearActuator::log_status()
{
    if (current_position != target_position || motor->get_speed())
        Logger::trace("ap:%ld, tp:%ld, ms:%d, md:%d, gl:%ld, st:%d", 
            current_position, target_position, motor->get_speed(), motor->get_direction(),
            get_limit(), stopped);
}

