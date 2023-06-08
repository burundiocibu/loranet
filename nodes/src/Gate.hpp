// -*- coding: utf-8 -*-
#include "Preferences.h"
#include "LinearActuator.hpp"

#ifndef GATE_HPP
#define GATE_HPP

// A class to translate actuator position into gate position
// gate position = 0 means fully closed
// gate position = 100 means fully open
class Gate
{
    public:
        Gate(LinearActuator* la_ptr, uint8_t lock_pin);
        void goto_position(float position);
        void open(double auto_close_dt = 0);
        void close();
        void stop();
        float get_position();
        bool update();
        bool set_closed_position(long position);
        long get_closed_position();
        String status();

    private:
        LinearActuator* actuator;
        Preferences gate_prefs;
        enum state_enum {state_closed, state_opening, state_open, state_closing} state, last_state;
        uint32_t last_open;   // When we last started an open
        double auto_close_time;  // when we should auto close the gate
        long closed_position;
        uint8_t lock_pin;
};
#endif