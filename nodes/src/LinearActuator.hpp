// -*- coding: utf-8 -*-
#include "md10c.hpp"

class LinearActuator
{
    public:
        LinearActuator(uint8_t encoder_pin, uint8_t limit_pin, MD10C* motor_ptr);
        static void encoder_isr();
        void goto_position(long position);
        long get_position() { return current_position; }
        bool get_speed() { return current_speed; }
        String get_status();

    private:
        uint8_t encoder_pin, limit_pin;
        static long current_position;
        static long target_position;
        static int current_speed;
        static int target_speed;
        static uint32_t update_time;
        static int last_speed;
        static MD10C* motor;
};
