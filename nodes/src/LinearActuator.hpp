// -*- coding: utf-8 -*-
#include "md10c.hpp"

#ifndef LINEAR_ACTUATOR_HPP
#define LINEAR_ACTUATOR_HPP

class LinearActuator
{
    public:
        LinearActuator(uint8_t _pulse_pin, uint8_t _limit_pin, MD10C* motor_ptr);
        void goto_position(long position);
        long get_position();
        void save_position();
        int get_speed();
        bool get_limit();
        String get_status();

    private:
        static uint8_t pulse_pin;
        static uint8_t limit_pin;
        volatile static long current_position;
        volatile static long target_position;
        volatile static bool limit;          // limit switch active
        volatile static uint32_t start_time; // time when motor started moving
        static MD10C* motor;
        volatile static bool dirty_position; // to indicate we got a valid stored position

        static void limit_isr();
        static void pulse_isr();


};
#endif