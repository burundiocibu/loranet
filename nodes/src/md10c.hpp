// -*- coding: utf-8 -*-
#include <Arduino.h>

// MD10C motor driver from cytron
// https://makermotor.com/pn00218-cyt2-cytron-13a-dc-motor-driver-md10c/
// https://makermotor.com/content/cytron/pn00218-cyt2/MD10C%20Rev2.0%20User%27s%20Manual.pdf

class MD10C
{
    public:
        MD10C(uint8_t pwm_pin, uint8_t direction_pin, uint8_t pwm_channel);
        MD10C() {};
        void run(int speed) { set_direction(speed); set_speed(abs(speed)); };
        void stop() { set_speed(0); };
        void set_speed(int speed);
        int get_speed() { return speed; };
        void set_direction(int dir);
        int get_direction() { return direction; }

    private:
        uint8_t pwm_pin, direction_pin; // pins used to talk to the driver
        uint8_t pwm_channel;
        int pwm_freq; // frequency of pwm, in Hz
        int speed; // -255..255
        int direction;
};