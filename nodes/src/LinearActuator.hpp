// -*- coding: utf-8 -*-
#include "md10c.hpp"

class LinearActuator
{
    public:
        LinearActuator(uint8_t _encoder_pin, uint8_t _limit_pin, MD10C* motor_ptr);
        void goto_position(long position);
        long get_position();
        int get_speed();
        bool get_limit_sw();
        void reset();
        String get_status();

    private:

};
