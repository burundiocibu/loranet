#include <Arduino.h>


class PeriodicTimer
{
    public:
        PeriodicTimer(uint32_t interval) : interval(interval), last_trigger(0) {};
        bool time();
        void set_interval(uint32_t new_interval) { interval = new_interval; }
    private:
        uint32_t interval, last_trigger;
};