// -*- coding: utf-8 -*-

// http://www.airspayce.com/mikem/arduino/RadioHead/
// http://www.airspayce.com/mikem/arduino/RadioHead/classRH__RF95.html
#include <RHReliableDatagram.h>
#include <RH_RF95.h>

// Note that this class should be made a singleton
class LoraNode
{
    public:
        const byte ledPin;  // LED to activate when running
        const uint8_t id;
        int tx_rtt = 0;
        RH_RF95 rf95;
        RHReliableDatagram manager;
        int txpwr = 20; // dBm, 5..23

        LoraNode(byte ledPin, uint8_t id);
        void led_ping(unsigned ms);

        long dt(unsigned long start_time);

        void send_msg(uint8_t dest, String& msg);
        virtual void run() = 0;
};
