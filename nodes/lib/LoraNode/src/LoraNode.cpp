// -*- coding: utf-8 -*-
#include <MacroLogger.h>
#include "LoraNode.hpp"
#include "utils.hpp"

#define SX1262_FREQ 915.0


volatile LoraNode::radio_state_enum LoraNode::radio_state = LoraNode::state_idle;
volatile int LoraNode::tx_dt = 0;
volatile uint32_t LoraNode::tx_start_time = 0;

LoraNode::LoraNode(uint8_t nss_pin, uint8_t dio_pin, uint8_t rst_pin, uint8_t busy_pin, uint8_t node_id) :
    node_id(node_id), radio(new Module(nss_pin, dio_pin, rst_pin, busy_pin)),
    tx_count(0), rx_count(0),
    rssi(0), snr(0)
{
    Serial.print(F("[SX1262] Initializing ... "));
    int state = radio.begin();
    if (state == RADIOLIB_ERR_NONE)
    {
        Serial.println(F("success!"));
    }
    else
    {
        Serial.print(F("failed, code "));
        Serial.println(state);
        while (true);
    }

    // Setup freq, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
    if (radio.setFrequency(SX1262_FREQ) == RADIOLIB_ERR_INVALID_FREQUENCY)
    {
        Serial.println(F("Selected frequency is invalid for this module!"));
        while (true);
    }

    // set bandwidth 
    if (radio.setBandwidth(125.0) == RADIOLIB_ERR_INVALID_BANDWIDTH) {
        Serial.println(F("Selected bandwidth is invalid for this module!"));
        while (true);
    }

    // set spreading factor to 7 
    if (radio.setSpreadingFactor(7) == RADIOLIB_ERR_INVALID_SPREADING_FACTOR) {
        Serial.println(F("Selected spreading factor is invalid for this module!"));
        while (true);
    }

    // set coding rate to 5
    if (radio.setCodingRate(5) == RADIOLIB_ERR_INVALID_CODING_RATE) {
        Serial.println(F("Selected coding rate is invalid for this module!"));
        while (true);
    }

    // set LoRa sync word
    // determined emperiacly to match the adafruit RF95 chip/driver
    // appears to be the RFM95W FSK default (register address 0x39)
    if (radio.setSyncWord(0x12) != RADIOLIB_ERR_NONE) {
        Serial.println(F("Unable to set sync word!"));
        while (true);
    }

    // set output power (accepted range is -17 - 22 dBm)
    if (radio.setOutputPower(txpwr) == RADIOLIB_ERR_INVALID_OUTPUT_POWER) {
        Serial.println(F("Selected output power is invalid for this module!"));
        while (true);
    }

    // set over current protection limit to 80 mA (accepted range is 45 - 240 mA)
    // NOTE: set value to 0 to disable overcurrent protection
    if (radio.setCurrentLimit(80) == RADIOLIB_ERR_INVALID_CURRENT_LIMIT) {
        Serial.println(F("Selected current limit is invalid for this module!"));
        while (true);
    }

    // set LoRa preamble length  (accepted range is 0 - 65535)
    if (radio.setPreambleLength(12) == RADIOLIB_ERR_INVALID_PREAMBLE_LENGTH) {
        Serial.println(F("Selected preamble length is invalid for this module!"));
        while (true);
    }

    radio.setDio1Action(LoraNode::radio_isr);
    radio.startReceive();
    radio_state = state_rx;
}

ICACHE_RAM_ATTR
void LoraNode::radio_isr()
{
    switch (radio_state)
    {
        case state_tx:
            tx_dt = dt(tx_start_time);
            radio_state = state_tx_complete;
            break;
        case state_rx:
            radio_state = state_rx_complete;
            break;
        case state_idle:
        case state_tx_complete:
        case state_rx_complete:
            break;
    }
}


bool LoraNode::get_message(String& msg, byte& sender)
{
    if (radio_state == state_tx_complete)
    {
        tx_count++;
        radio.finishTransmit();
        radio.startReceive();
        radio_state = state_rx;
        Logger::trace("state_tx_complete");

    }
    else if (radio_state == state_rx_complete)
    {
        int pl = radio.getPacketLength(true);
        byte buff[pl+1];
        int state = radio.readData(buff, pl);
        radio.startReceive();
        radio_state = state_rx;

        sender = buff[1];
        buff[pl] = 0;
        msg = String((char*)buff + 4);

        rx_count++;
        rssi = radio.getRSSI();
        snr = radio.getSNR();
        if (state == RADIOLIB_ERR_NONE)
            return true;
        Logger::trace("state_rx_complete");
    }
    return false;
}


bool LoraNode::send_msg(uint8_t dest, const String& msg)
{
    if (radio_state == state_tx)
        return false;
    uint8_t buff[4+msg.length()];
    buff[0] = dest;
    buff[1] = node_id;
    buff[2] = 1;  // message_id
    buff[3] = 0;  // flags Bit 7: Ack packet, Bit 6 packet is a retry
    memcpy(buff+4, msg.c_str(), msg.length());
    radio_state = state_tx;
    radio.startTransmit(buff, sizeof(buff));
    tx_start_time = millis();
    Logger::trace("tx: to:%d, %s", dest, msg.c_str());
    return true;
}


String LoraNode::status()
{
    String msg = "state:";
    switch(radio_state)
    {
        case state_idle:        msg += "idle"; break;
        case state_tx:          msg += "tx"; break;
        case state_tx_complete: msg += "txc"; break;
        case state_rx:          msg += "rx"; break;
        case state_rx_complete: msg += "rxc"; break;
    }
    msg += ", tx_dt:" + String(tx_dt) +
        ", rssi:" + String(rssi) +
        ", snr:" + String(snr) +
        ", tx_count:" + String(tx_count) +
        ", rx_count:" + String(rx_count);
    return msg;
}