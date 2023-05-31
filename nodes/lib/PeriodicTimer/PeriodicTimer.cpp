#include "PeriodicTimer.hpp"


bool PeriodicTimer::time()
{
    uint32_t now = millis();
    if (last_trigger == 0)
    {
        last_trigger = now;
        return true;
    }

    long dt = now - last_trigger;
    if (dt < 0)
        dt += 0x10000;

    if (dt > interval)
    {
        last_trigger = now;
        return true;
    }
    return false;
}
