// -*- coding: utf-8 -*-
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>

// watchdog interrupt
ISR (WDT_vect)
{
    wdt_disable();
}


void deep_sleep(unsigned long msec)
{
#ifdef DISABLE_SLEEP
    delay(msec);
    return;
#endif

    unsigned remaining = msec;
    while (remaining)
    {
        if (remaining < 16)
        {
            delay(remaining);
            remaining = 0;
        }
        else
        {
            // clear various "reset" flags
            MCUSR = 0;
            byte prescale = 0;
            if      (remaining >= 8000) {prescale = bit(WDP3)|                    bit(WDP0); remaining -=8000; }
            else if (remaining >= 4000) {prescale = bit(WDP3)                              ; remaining -=4000; }
            else if (remaining >= 2000) {prescale =           bit(WDP2)|bit(WDP1)|bit(WDP0); remaining -=2000; }
            else if (remaining >= 1000) {prescale =           bit(WDP2)|bit(WDP1)          ; remaining -=1000; }
            else if (remaining >= 500)  {prescale =           bit(WDP2)|          bit(WDP0); remaining -=500; }
            else if (remaining >= 250)  {prescale =           bit(WDP2)                    ; remaining -=250; }
            else if (remaining >= 125)  {prescale =                     bit(WDP1)|bit(WDP0); remaining -=125; }
            else if (remaining >= 64)   {prescale =                     bit(WDP1)          ; remaining -=64; }
            else if (remaining >= 32)   {prescale =                               bit(WDP0); remaining -=32; }
            else if (remaining >= 16)   {remaining -=16;}

            // These two register accesses have to be adjacent
            WDTCSR = bit (WDCE) | bit (WDE);
            WDTCSR = bit(WDIE) | prescale;
            wdt_reset();  // pat the dog

            set_sleep_mode (SLEEP_MODE_PWR_DOWN);
            sleep_enable();
            sleep_cpu();
            sleep_disable();
        }
    }
}
