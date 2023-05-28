// -*- coding: utf-8 -*-
#include "md10c.hpp"

class LinearActuator
{
    public:
        LinearActuator(uint8_t _pulse_pin, uint8_t _limit_pin, MD10C* motor_ptr);
        void goto_position(long position);
        long get_position();
        int get_speed();
        bool get_limit();
        String get_status();

    private:
        static uint8_t pulse_pin;
        static uint8_t limit_pin;
        static long current_position;
        static long target_position;
        static bool limit;          // limit switch active
        static bool zero_found;
        static uint32_t start_time; // time when motor started moving
        static MD10C* motor;

        static void limit_isr();
        static void pulse_isr();
};
