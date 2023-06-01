#include <Arduino.h>


class PeriodicTimer
{
    public:
        PeriodicTimer(uint32_t interval) : interval(interval), last_trigger(0) {};
        bool time();
    private:
        uint32_t interval, last_trigger;
};