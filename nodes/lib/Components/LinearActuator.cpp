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
volatile uint32_t LinearActuator::start_time = 0;
MD10C *LinearActuator::motor;
volatile bool LinearActuator::dirty_position = false;
volatile uint32_t LinearActuator::last_pcnt = 0;
volatile uint32_t LinearActuator::last_pcnt_micros = 0;
volatile int LinearActuator::last_open_error = 0;

#define SF_LIMIT_ISR 0
#define SF_LIMIT_STOP 1
#define SF_MOTOR_STOP 2
#define SF_PULSE_TIMEOUT 4
#define SF_PHANTOM_PULSE 5
#define SF_MOTOR_HUNT 6
#define SF_ARMATTARGET 8
#define SF_DIRTY_POSITION 9
volatile uint32_t LinearActuator::status_flags = 0;

Preferences preferences;

#define PCNT_UNIT PCNT_UNIT_0

const int min_speed = 96;


/// @brief constructor for LinearActuator
/// @param _pulse_pin : pin connected to encoder open collector output
/// @param _limit_pin : pin connected to limit switch open collector output
/// @param motor_ptr
LinearActuator::LinearActuator(uint8_t _pulse_pin, uint8_t _limit_pin, MD10C *motor_ptr)
{
    pulse_pin = _pulse_pin;
    limit_pin = _limit_pin;
    pinMode(pulse_pin, INPUT);
    pinMode(limit_pin, INPUT_PULLUP);
    motor = motor_ptr;
    motor->stop();

    int flag_value = -1;
    preferences.begin("LinearActuator");
    current_position = preferences.getInt("position", flag_value);
    preferences.end();
    target_position = current_position;

    if (current_position == flag_value)
        goto_position(0);

    attachInterrupt(digitalPinToInterrupt(limit_pin), limit_isr, RISING);

    pcnt_config_t pcnt_config = {};         // Instance of pulse counter
    pcnt_config.pulse_gpio_num = pulse_pin; // pin assignment for pulse counter = GPIO 15
    pcnt_config.pos_mode = PCNT_COUNT_INC;  // count rising edges (=change from low to high logical level) as pulses
    pcnt_config.counter_h_lim = 0xffff;     // set upper limit of counting
    pcnt_config.unit = PCNT_UNIT;           // select ESP32 pulse counter unit 0
    pcnt_config.channel = PCNT_CHANNEL_0;   // select channel 0 of pulse counter unit 0
    pcnt_unit_config(&pcnt_config);         // configur rigisters of the pulse counter

    pcnt_counter_pause(PCNT_UNIT);          // pause pulse counter unit
    pcnt_counter_clear(PCNT_UNIT);          // zero and reset of pulse counter unit
    pcnt_set_filter_value(PCNT_UNIT, 1000); // set glitch filter, units of ns
    pcnt_filter_enable(PCNT_UNIT);          // enable counter glitch filter (damping)
    pcnt_counter_resume(PCNT_UNIT);         // resume counting on pulse counter unit

    // A 1khz timer to firer the timer isr
    hw_timer_t *timer = NULL;
    timer = timerBegin(0, 80, true);               // the 80 prescaller gets the clock down 1MHz
    timerAttachInterrupt(timer, &timer_isr, true); // attach the ISR and trigger on edgess
    timerAlarmWrite(timer, 1000, true);            // trigger on count=1k (1ms)
    timerAlarmEnable(timer);                       // GOOOOOO
}

// handle the timer expiring
void LinearActuator::timer_isr()
{
    int16_t pcnt;
    pcnt_get_counter_value(PCNT_UNIT, &pcnt);
    int pcnt_delta = pcnt - last_pcnt;
    long pcnt_dt = micros() - last_pcnt_micros;
    if (pcnt_dt < 0)
        pcnt_dt += 0x10000;

    if (pcnt_delta == 0)
    {
        if (pcnt_dt > 5e6 && motor->get_speed())
        {
            stop();
            status_flags |= _BV(SF_PULSE_TIMEOUT) | _BV(SF_MOTOR_STOP);
        }
    }
    else
    {
        if (pcnt_dt < 4000)
        {
            status_flags |= _BV(SF_PHANTOM_PULSE);
            last_pcnt = pcnt;
            last_pcnt_micros = micros();
        }
        else
        {
            current_position += motor->get_direction() * pcnt_delta;
            status_flags |= _BV(SF_DIRTY_POSITION);
            dirty_position = true;
            last_pcnt = pcnt;
            last_pcnt_micros = micros();
        }
    }

    bool limit = digitalRead(limit_pin);
    int speed = motor->get_speed();
    int err = target_position - current_position;

    if (limit)
    {
        if (target_position > 0 && err >= 0)
            err = err;
        else
        {
            last_open_error = current_position;
            current_position = 0;
            stop();
            status_flags |= _BV(SF_LIMIT_STOP);
            return;
        }
    }
    else
    {
        if (target_position > 0)
            err = err;
        else if (target_position == 0)
        {
            if ((speed < 0 && err < 0) ||
                (speed == 0 && err > 0) ||
                (speed > 0 && err < 0))
                err = err;
            else
                err = -min_speed;
        }
        else
        {
            if (err < 0)
                err = err;
            else
                err = -min_speed;
        }
    }

    if (motor->get_speed() == 0 && status_flags & _BV(SF_ARMATTARGET))
        status_flags |= _BV(SF_MOTOR_HUNT);

    int new_speed = 0;
    if (err > 0)
    {
        new_speed = min( 2*err, 255);
        new_speed = min(int(dt(start_time)/8), new_speed);
        new_speed = max( new_speed, min_speed); 
    }
    else if (err < 0)
    {
        new_speed = max( 2*err, -255);
        new_speed = max(new_speed, -int(dt(start_time)/8));
        new_speed = min( new_speed, -min_speed); 
    }

    motor->set_speed(new_speed);
}

// handle the limit switch being triggered
void LinearActuator::limit_isr()
{
    if (motor->get_speed() < 0)
    {
        last_open_error = current_position;
        current_position = 0;
        stop();
        status_flags |= _BV(SF_LIMIT_ISR);
    }
}

void LinearActuator::stop()
{
    motor->stop();
    // yeah this could go into a pcnt isr...
    pcnt_counter_pause(PCNT_UNIT);  // pause pulse counter unit
    pcnt_counter_clear(PCNT_UNIT);  // zero and reset of pulse counter unit
    pcnt_counter_resume(PCNT_UNIT); // resume counting on pulse counter unit
    last_pcnt = 0;
    target_position = current_position;
}

// Fully retracting arm is position 0, limit switch engaged.
// Fully extending arm is max pos, estimated.
void LinearActuator::goto_position(long position)
{
    status_flags = 0;

    if (position > 2250)
        return;

    if (position <= 0 && digitalRead(limit_pin))
    {
        current_position = 0;
        target_position = 0;
        save_position();
        return;
    }

    target_position = position;
    last_pcnt_micros = micros(); // reset the pulse timeout
    start_time = millis();
}

void LinearActuator::save_position()
{
    if (!dirty_position || motor->get_speed())
        return;
    preferences.begin("LinearActuator");
    preferences.putInt("position", current_position);
    dirty_position = false;
    preferences.end();
    Logger::info("position saved %d", current_position);
    status_flags &= ~_BV(SF_DIRTY_POSITION);
}

bool LinearActuator::update()
{
    static uint32_t last_update_time = 0;
    long t = dt(last_update_time);
    static long last_position = 0;
    static long last_status_flags = 0;
    bool changed = motor->get_speed() || current_position != last_position || status_flags != last_status_flags;
    if (changed && t > 250)
    {
        last_position = current_position;
        last_update_time = millis();
        last_status_flags = status_flags;
        return true;
    }
    else
        return false;
}

String LinearActuator::status()
{
    String msg;
    msg =
        "ap:" + String(current_position) +
        ",at:" + String(target_position) +
        ",al:" + String(digitalRead(limit_pin)) +
        ",ms:" + String(motor->get_speed()) +
        ",aloe:" + String(last_open_error) +
        ",asf:" + String(status_flags, HEX);
    return msg;
}
