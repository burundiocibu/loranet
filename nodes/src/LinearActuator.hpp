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
        static void stop();
        int get_speed();
        bool get_limit();
        bool get_stopped();
        void log_status();

    private:
        static uint8_t pulse_pin;
        static uint8_t limit_pin;
        volatile static long current_position;
        volatile static long target_position;
        volatile static bool limit;          // limit isr tripped
        volatile static uint32_t start_time; // time when motor started moving
        static MD10C* motor;
        volatile static bool dirty_position; // to indicate we got a valid stored position
        volatile static bool stopped;
        volatile static uint32_t last_pcnt;
        volatile static uint32_t last_pcnt_micros;

        static void timer_isr();
        static void limit_isr();

};
#endif