#include "PeriodicTimer.hpp"


bool PeriodicTimer::time()
{
    if (last_trigger == 0)
    {
        last_trigger = millis();
        return true;
    }

    long dt = millis() - last_trigger;
    if (dt < 0)
        dt += 0x10000;

    if (dt > interval)
    {
        last_trigger += interval;
        return true;
    }
    return false;
}
