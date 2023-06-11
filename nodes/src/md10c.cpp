// -*- coding: utf-8 -*-
#include "md10c.hpp"


/// @brief MD10C constructor
/// @param pwm pin for pwm
/// @param dir pin for drive direction
MD10C::MD10C(uint8_t pwm_pin, uint8_t direction_pin, uint8_t pwm_channel):
    pwm_pin(pwm_pin), direction_pin(direction_pin), pwm_channel(pwm_channel)
{
    pwm_freq = 10000; // Hz
    ledcSetup(pwm_channel, pwm_freq, 8);
    ledcAttachPin(pwm_pin, pwm_channel);
    pinMode(direction_pin, OUTPUT);
    
    set_direction(1);
    set_speed(0);
}

/// @brief  sets the motor direction but not speed
/// @param _speed used to determine direction, <0 means towards limit switch
void MD10C::set_direction(int _speed)
{
    if (_speed > 0)
    {
        direction = 1;
        digitalWrite(direction_pin, HIGH);
    }
    else
    {
        direction = -1;
        digitalWrite(direction_pin, LOW);
    }
}

/// @brief sets the motor speed but not direction
/// @param _speed speed, 0=stopped, 255=maximum
void MD10C::set_speed(int _speed)
{
    speed = _speed;
    ledcWrite(pwm_channel, speed);
    last_speed_millis = millis();
}
