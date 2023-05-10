// -*- coding: utf-8 -*-

#include "tb67h420.hpp"

/// @brief TB67H420 constructor
/// @param pwma pin for pwma
/// @param ina1 pin for ina1
/// @param ina2 pin for ina2
/// @param lo1  pin for lo1
/// @param lo2  pin for lo2
TB67H420::TB67H420(uint8_t pwma, uint8_t ina1, uint8_t ina2, uint8_t lo1, uint8_t lo2):
    pwma(pwma), ina1(ina1), ina2(ina2), lo1(lo1), lo2(lo2)
{
    pinMode(pwma, OUTPUT);
    digitalWrite(pwma, LOW);
    pinMode(ina1, OUTPUT);
    digitalWrite(ina1, LOW);
    pinMode(ina2, OUTPUT);
    digitalWrite(ina2, LOW);
    pinMode(lo1, INPUT);
    pinMode(lo2, INPUT);
    
    // PWM is being generated on Arduino pin 5
    // 32u4 pin 31, port C6, OC.3A
    // Using Timer 3 output compare A

    // See tables 14-3 and 14-5
    // COM3A[1:0] = 10 - Clear OC3A on compare match, Set OC3A at TOP
    // WGM3[3:0] = 111 - Fast PWM, 10-bit, TOP=0x03FF TOP TOP
    TCCR3A = _BV(COM3A1) | _BV(WGM32) | _BV(WGM31) | _BV(WGM30);
    
    // Table 14-6
    // CS0[2:0] = 011, /64 prescaler, about 61 Hz
    TCCR3B = _BV(CS31) | _BV(CS30);

    pwm_top = 0x3ff;
    pwm_period = 64 / 3.91; // ms
    set_pwm_duty(0);
}

void TB67H420::run(int speed)
{
    if (speed > 0)
    {
        set_pwm_duty(speed);
        digitalWrite(lo2, LOW);
        digitalWrite(lo1, HIGH);
    }
    else if (speed < 0)
    {
        set_pwm_duty(-speed);
        digitalWrite(lo1, LOW);
        digitalWrite(lo2, HIGH);
    }
    else
    {
        set_pwm_duty(speed);
        digitalWrite(lo1, LOW);
        digitalWrite(lo2, LOW);
    }
}

int TB67H420::error()
{
    return ~digitalRead(lo1) | ~digitalRead(lo2) << 1;
}