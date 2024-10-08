// -*- coding: utf-8 -*-
#include <MacroLogger.h>
#include "Gate.hpp"
#include "utils.hpp"


Gate::Gate(LinearActuator* la_ptr, uint8_t lock_pin) : 
    actuator(la_ptr),
    auto_close_time(0),
    lock_pin(lock_pin),
    last_open(0)
{
    gate_prefs.begin("Gate");
    closed_position = gate_prefs.getLong("closed_position", 1000);
    gate_prefs.end();
    Logger::info("loaded closed_position:%d", closed_position);

    pinMode(lock_pin, OUTPUT);
    digitalWrite(lock_pin, HIGH);

}

/// @brief starts gate to go to given position
/// @param position 0=closed, 100=fully open
void Gate::goto_position(float position)
{
    if (get_position() < 2)
    {
        digitalWrite(lock_pin, LOW);
        Logger::trace("unlock");
    }
    actuator->goto_position(closed_position-closed_position*position/100);
};


/// @brief gets current gate position
/// @return gate postion, 0=closed, 100=fully open
float Gate::get_position()
{
    return 100*(closed_position - actuator->get_position())/closed_position;
};


/// @brief stiops gate motion where it is
void Gate::stop()
{
    actuator->stop();
};


bool Gate::set_closed_position(long position)
{
    closed_position = position;
    gate_prefs.begin("Gate");
    gate_prefs.putLong("closed_position", closed_position);
    gate_prefs.end();
    Logger::info("saved closed_position:%d", closed_position);
    return true;
};


long Gate::get_closed_position()
{
    return closed_position;
}

/// @brief opens gate, if it is not
/// @param auto_close_dt seconds till gate shall auto close, 0 means never auto close
void Gate::open(double auto_close_dt)
{
    if (dt(last_open) < 1000 || get_position() > 95 )
        return;
    last_open = millis();
    Logger::trace("open");
    goto_position(100);
    if (auto_close_dt != 0)
        auto_close_time = uptime() + auto_close_dt;
    else
        auto_close_time = 0;
}


/// @brief starts gate closing
void Gate::close()
{
    Logger::trace("close");
    goto_position(0);
    auto_close_time = 0;
}


/// @brief checks state of gate and performs auto-closing if needed 
/// @return true of the state of the gate has changed and/or an update needs to be sent
bool Gate::update()
{
    if (auto_close_time != 0 && uptime() > auto_close_time)
        close();

    int gp = get_position();
    int lock = digitalRead(lock_pin);
    if (lock && gp < 2 && actuator->get_speed() )
    {
        digitalWrite(lock_pin, LOW);
        Logger::trace("unlock");
    }
    else if (!lock && gp > 3)
    {
        digitalWrite(lock_pin, HIGH);
        Logger::trace("lock");
    }
 
    actuator->save_position();

    if (actuator->update())
        return true;
    else
        return false;
}


String Gate::status()
{
    String msg;
    msg = "gp:" + String(int(get_position()))
        + ",acp:" + String(closed_position)
        + "," + actuator->status();
    return msg;
}
