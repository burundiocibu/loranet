// -*- coding: utf-8 -*-

#include "md10c.hpp"

/// @brief MD10C constructor
/// @param pwm pin for pwm
/// @param dir pin for drive direction
MD10C::MD10C(uint8_t pwm_pin, uint8_t direction_pin, uint8_t encoder_pin):
    pwm_pin(pwm_pin), direction_pin(direction_pin), encoder_pin(encoder_pin)
{
    pinMode(pwm_pin, OUTPUT);
    digitalWrite(pwm_pin, LOW);
    pinMode(direction_pin, OUTPUT);
    digitalWrite(direction_pin, LOW);
    pinMode(encoder_pin, INPUT);
    
    // PWM is being generated on Feather pin 5 
    // 32u4 pin 31, port C6, OC.3A
    // Using Timer 3 output compare A

    // See tables 14-3 and 14-5
    // COM3A[1:0] = 10 - Clear OC3A on compare match, Set OC3A at TOP
    // WGM3[3:0] = 111 - Fast PWM, 10-bit, TOP=0x03FF TOP TOP
    // TCCR3A = _BV(COM3A1) | _BV(WGM32) | _BV(WGM31) | _BV(WGM30);
    // WGM3[3:0] = 101 Fast PWM, 8-bit, top=0xff
    TCCR3A = _BV(COM3A1) | _BV(WGM32) |_BV(WGM30);
    
    // Table 14-6
    // Set prescaler to zero to stop timer
    TCCR3B = 0;

    pwm_top = 0xff;
    pwm_freq = 15600; // Hz
    set_pwm_duty(0);
}


void MD10C::set_pwm_duty(int _pwm_duty)
{
    pwm_duty = min(100, _pwm_duty);
    // Set the duty cycle
    OCR3A = uint16_t((pwm_duty/100.0) * pwm_top);
    // Start prescaler to /1, about 3.9 kHz to start the timer
    TCCR3B = _BV(CS30);
}


void MD10C::run(int speed)
{
    if (speed > 0)
    {
        set_pwm_duty(speed);
        digitalWrite(direction_pin, LOW);
    }
    else if (speed < 0)
    {
        set_pwm_duty(-speed);
        digitalWrite(direction_pin, HIGH);
    }
    else
    {
        set_pwm_duty(0);
    }
}
