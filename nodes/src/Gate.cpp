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
    digitalWrite(lock_pin, LOW);

}

/// @brief starts gate to go to given position
/// @param position 0=closed, 100=fully open
void Gate::goto_position(float position)
{
    if (get_position() < 1)
    {
        digitalWrite(lock_pin, HIGH);
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
    if (dt(last_open) < 1000)
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
    if (gp < 2 && gp > 0 && !digitalRead(lock_pin))
    {
        digitalWrite(lock_pin, HIGH);
        Logger::trace("unlock");
    }
    else if (gp > 3 && digitalRead(lock_pin))
    {
        digitalWrite(lock_pin, LOW);
        Logger::trace("lock");
    }
    else if (gp == 0 && digitalRead(lock_pin) && actuator->get_speed() == 0)
    {
        digitalWrite(lock_pin, LOW);
        Logger::trace("lock");
    }

    actuator->save_position();
    actuator->log_status();

    static uint32_t last_update_time = 0;
    long t = dt(last_update_time);
    static long last_pos = 0;
    long current_pos = actuator->get_position();
    if ( t > 60000 || (current_pos != last_pos && t > 250))
    {
        last_pos = current_pos;
        last_update_time = millis();
        return true;
    }
    else
        return false;
}


String Gate::status()
{
    String msg;
    msg += String("gp:") + String(long(get_position()));
    msg += String(",ap:") + String(actuator->get_position());
    msg += String(",acp:") + String(get_closed_position());
    msg += (String(",al:") + String(actuator->get_limit()));
    msg += (String(",loe:") + String(actuator->get_last_open_error()));
    return msg;
}
