// -*- coding: utf-8 -*-
#include <Arduino.h>

// MD10C motor driver from cytron
// https://makermotor.com/pn00218-cyt2-cytron-13a-dc-motor-driver-md10c/
// https://makermotor.com/content/cytron/pn00218-cyt2/MD10C%20Rev2.0%20User%27s%20Manual.pdf

class MD10C
{
    public:
        MD10C(uint8_t pwm_pin, uint8_t direction_pin);
        MD10C() {};
        void run(int speed);
        void stop() { run(0); };
        int get_speed();
        int get_dir();

    private:
        uint8_t pwm_pin, direction_pin; // pins used to talk to the driver
        int pwm_top;  // max count of OC reg
        int pwm_freq; // frequency of pwm, in Hz
        int motor_speed; // -100..100
        int previous_speed;
        void set_pwm_duty(int _pwm_duty);
};