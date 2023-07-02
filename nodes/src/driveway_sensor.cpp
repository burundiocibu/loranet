// -*- coding: utf-8 -*-
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>

#include "driveway_sensor.hpp"
#include "PeriodicTimer.hpp"
#include "utils.hpp"


void isr0()
{
    sleep_disable();
}

void power_down()
{
    rf95.sleep();
    Serial1.println("Powering Down");
    delay(250);
    cli();
    attachInterrupt(digitalPinToInterrupt(WAKEUP_PIN), isr0, RISING);
    sei();
    ADCSRA &= ~(1 << ADEN);//adc off
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_cpu();

    ADCSRA |= (1 << ADEN); //adc back on
}


void loop()
{
    power_down();

    double vbat = analogRead(VBAT) * 2 * 3.3 / 1024;

    String msg = "ut:" + String(int(uptime())) +
        ",vb:" + String(vbat,2);

    int tx_count = 0;
    while (tx_count < 4)
    {
        Serial1.print("tx " + String(tx_count) + ":");
        Serial1.println(msg);
        rf95.send(msg.c_str(), msg.length());
        rf95.waitPacketSent();
        tx_count++;

        if (rf95.waitAvailableTimeout(500))
        {
            uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
            uint8_t len = sizeof(buf);
            if (rf95.recv(buf, &len))
            {
                Serial1.print("got reply: ");
                Serial1.print((char *)buf);
                Serial1.print(", RSSI: ");
                Serial1.println(rf95.lastRssi(), DEC);
            }
            break;
        }
        else
            Serial1.println("Rx Timeout");
    }
}