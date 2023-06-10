// -*- coding: utf-8 -*-
#include <MacroLogger.h>
#include "Preferences.h"
#include "driver/pcnt.h"

#include "LinearActuator.hpp"
#include "utils.hpp"


uint8_t LinearActuator::pulse_pin = 0;
uint8_t LinearActuator::limit_pin = 0;
volatile long LinearActuator::current_position = 0;
volatile long LinearActuator::target_position = 0;
volatile bool LinearActuator::limit = false;
volatile uint32_t LinearActuator::start_time = 0;
MD10C* LinearActuator::motor;
volatile bool LinearActuator::dirty_position = false;
volatile bool LinearActuator::stopped = true;
volatile uint32_t LinearActuator::last_pcnt = 0;
volatile uint32_t LinearActuator::last_pcnt_micros = 0;

Preferences preferences;

#define PCNT_UNIT PCNT_UNIT_0

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

    attachInterrupt(digitalPinToInterrupt(limit_pin), limit_isr, RISING);

    pcnt_config_t pcnt_config = { };                // Instance of pulse counter
    pcnt_config.pulse_gpio_num = pulse_pin ;        // pin assignment for pulse counter = GPIO 15
    pcnt_config.pos_mode = PCNT_COUNT_INC;          // count rising edges (=change from low to high logical level) as pulses
    pcnt_config.counter_h_lim = 0xffff;             // set upper limit of counting 
    pcnt_config.unit = PCNT_UNIT;                   // select ESP32 pulse counter unit 0
    pcnt_config.channel = PCNT_CHANNEL_0;           // select channel 0 of pulse counter unit 0
    pcnt_unit_config(&pcnt_config);                 // configur rigisters of the pulse counter
  
    pcnt_counter_pause(PCNT_UNIT);             // pause pulse counter unit
    pcnt_counter_clear(PCNT_UNIT);             // zero and reset of pulse counter unit
    pcnt_set_filter_value(PCNT_UNIT, 1000);    // set glitch filter, units of ns
    pcnt_filter_enable(PCNT_UNIT);             // enable counter glitch filter (damping)
    pcnt_counter_resume(PCNT_UNIT);            // resume counting on pulse counter unit

    // A 1khz timer to firer the timer isr
    hw_timer_t * timer = NULL;
    timer = timerBegin(0, 80, true);   // the 80 prescaller gets the clock down 1MHz
    timerAttachInterrupt(timer, &timer_isr, true);
    timerAlarmWrite(timer, 1000, true);
    timerAlarmEnable(timer);
}


// handle the timer expiring
void LinearActuator::timer_isr()
{
    if (digitalRead(limit_pin) && target_position <= 0)
    {
        stop();
        target_position = current_position = 0;
    }

    int16_t pcnt;
    pcnt_get_counter_value(PCNT_UNIT, &pcnt);
    int pcnt_delta = pcnt - last_pcnt;
    int pcnt_dt = micros() - last_pcnt_micros;
    if (pcnt_dt < 0)
        pcnt_dt += 0x10000;

    if (pcnt_delta == 0 || pcnt_dt < 4000)
        return;

    current_position += motor->get_direction() * pcnt_delta;
    last_pcnt = pcnt;
    last_pcnt_micros = micros();
    dirty_position = true;

    int err = target_position - current_position;
    if (err == 0)
    {
        stop();
        return;
    }
        
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


// handle the limit switch being triggered
void LinearActuator::limit_isr()
{
    stop();
    limit = true;
    current_position = 0;
}


long LinearActuator::get_position() { return current_position; }
int LinearActuator::get_speed() { return motor->get_speed(); }
bool LinearActuator::get_limit() { return limit = digitalRead(limit_pin); }
bool LinearActuator::get_stopped() { return stopped; }


void LinearActuator::stop()
{
    motor->stop();
    stopped = true;
    // yeah this could go into a pcnt isr...
    pcnt_counter_pause(PCNT_UNIT);             // pause pulse counter unit
    pcnt_counter_clear(PCNT_UNIT);             // zero and reset of pulse counter unit
    pcnt_counter_resume(PCNT_UNIT);            // resume counting on pulse counter unit
    last_pcnt = 0;
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
        if (current_position <= 0)
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
    int16_t pulse_count;
    pcnt_get_counter_value(PCNT_UNIT, &pulse_count);

    static int last_current_position = 0;
    if (current_position != last_current_position || motor->get_speed())
    {
        Logger::info("ap:%ld, tp:%ld, ms:%d, md:%d, gl:%ld, st:%d, pc:%d", 
            current_position, target_position, motor->get_speed(), motor->get_direction(),
            get_limit(), stopped, pulse_count);
        last_current_position = current_position;
    }
}

