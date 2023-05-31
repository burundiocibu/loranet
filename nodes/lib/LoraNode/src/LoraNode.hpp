// -*- coding: utf-8 -*-
#include "RadioLib.h"


// A wrapper around RadioLib to do non-blocking comms
class LoraNode
{
    public:
        LoraNode(uint8_t nss_pin, uint8_t dio_pin, uint8_t rst_pin, uint8_t busy_pin, uint8_t node_id);
        bool send_msg(uint8_t dest, const String& msg);
        bool get_message(String& msg, byte& sender);
        enum radio_state_enum {
            state_idle, state_rx, state_rx_complete, state_tx, state_tx_complete};
        radio_state_enum get_radio_state() { return radio_state; }
        int get_rssi() { return rssi; }
        int get_snr() { return snr; }
        int get_tx_dt() { return tx_dt; }
        String status();
   
    private:
        const uint8_t node_id;
        volatile static int tx_dt;
        volatile static uint32_t tx_start_time;
        volatile static radio_state_enum radio_state;
        uint32_t rx_count, tx_count;
        float rssi, snr;
        SX1262 radio;
        int txpwr = 17; // dBm, 5..21
        static void radio_isr();
};
