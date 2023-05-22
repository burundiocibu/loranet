// -*- coding: utf-8 -*-
#include <Arduino.h>


// returns uptime in seconds
double uptime()
{
    static uint32_t last_millis = 0;
    static uint32_t wraps = 0;
    uint32_t ms = millis();
    if (ms < last_millis)
        wraps++;
    return 0.001 * ms + wraps * 4294967.0; // 4294967 = 2^32/1000
}


String runtime()
{
    double sec = uptime();
    long hour = sec / 3600;
    sec -= hour*3600;
    long min = sec/60;
    sec -= min*60;
    int ms = 1000*(sec - int(sec));

    char buff[15];
    sprintf(buff, "%ld:%02ld:%02d.%03d", hour, min, int(sec), ms);
    return String(buff);
}


long dt(unsigned long start_time)
{
    unsigned long now = millis();
    long dt = now - start_time;
    if (dt < 0)
        dt += 0x10000;
    return dt;
}
