// -*- coding: utf-8 -*-
#include "Gate.hpp"


Gate::Gate(LinearActuator* la_ptr) : 
    actuator(la_ptr)
{
    static int flag_value = 500;
    gate_prefs.begin("Gate");
    closed_position = gate_prefs.getLong("closed_position", flag_value);
    gate_prefs.end();
}

void Gate::goto_position(float position)
{
    actuator->goto_position(closed_position-closed_position*position/100);
};

float Gate::get_position()
{
    return 100*(closed_position - actuator->get_position())/closed_position;
};

int Gate::get_speed()
{
    return actuator->get_speed();
};

bool Gate::set_closed_position(long position)
{
    closed_position = position;
    gate_prefs.begin("Gate");
    gate_prefs.putLong("closed_position", closed_position);
    gate_prefs.end();
    return true;
};

long Gate::get_closed_position()
{
    return closed_position;
}