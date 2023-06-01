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
        Gate(LinearActuator* la_ptr);
        void goto_position(float position);
        float get_position();
        int get_speed();
        bool set_closed_position(long position);
        long get_closed_position();

    private:
        LinearActuator* actuator;
        Preferences gate_prefs;
        long closed_position;
};
#endif