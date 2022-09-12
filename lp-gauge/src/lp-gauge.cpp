// -*- coding: utf-8 -*-
// This is for a DIYmall LoRa32u4 board; appears quite similiar to the Adafruit 32u4 lora feather
#include <SPI.h>

// http://www.airspayce.com/mikem/arduino/RadioHead/
// http://www.airspayce.com/mikem/arduino/RadioHead/classRH__RF95.html
#include <RHReliableDatagram.h>
#include <RH_RF95.h>

#include "OneWireWrapper.hpp"

//#define DISABLE_SLEEP
#include "deep_sleep.hpp"

// 32u4 i/o assignments
#define RFM95_RST 4
#define DS18B20_PWR 6
#define RFM95_INT 7
#define RFM95_CS 8
#define VBAT 9  // connected to Vbatt through a divider network
#define AH49E_OUT 10
#define AH49E_GND 11
#define DS18B20_D0 12
#define USER_LED 13
#define LIPO_CHARGER_EN PB0 // must be brough high to enable measuring or charging the battery


// Note that this class should be made a singleton
class LoraNode
{
    public:
        const byte ledPin;  // LED to activate when running
        const uint8_t id;
        int tx_rtt = 0;
        RH_RF95 rf95;
        RHReliableDatagram manager;
        int txpwr = 13; // dBm, 5..23

        LoraNode(byte ledPin, uint8_t id) :
            ledPin(ledPin), id(id),
            rf95(RFM95_CS, RFM95_INT),
            manager(rf95, id)
        {
            pinMode(ledPin, OUTPUT);
            led_ping(1);
            pinMode(RFM95_RST, OUTPUT);
            digitalWrite(RFM95_RST, HIGH);


            Serial.println("LoRaNet node");

            if (!manager.init())
                Serial.println("manager init failed");

            // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
            while (!rf95.init())
            {
                Serial.println("LoRa radio init failed");
                while (1);
            }
            Serial.println("LoRa radio init OK!");

            if (!rf95.setFrequency(915.0))
            {
                Serial.println("setFrequency failed");
                while (1);
            }
        }

        void led_ping(unsigned ms)
        {
            digitalWrite(ledPin, HIGH);
            delay(ms);
            digitalWrite(ledPin, LOW);
        }

        long dt(unsigned long start_time)
        {
            unsigned long now = millis();
            if (now < start_time)
                return now - start_time + 0x10000;
            else
                return now - start_time;
        }

        void send_msg(uint8_t dest, String& msg)
        {
            Serial.println("Tx to:" + String(dest) + ", msg:" + msg);
            rf95.setTxPower(txpwr);
            unsigned long t0 = millis();
            if (!manager.sendtoWait((uint8_t*)msg.c_str(), msg.length(), dest))
                Serial.println(" sendtoWait failed");
            tx_rtt = dt(t0);
        }

        virtual void run() = 0;
};


class LPGauge : public LoraNode
{
    private:
        OneWire ds;
        float vbat;
        float ah49e_out;
        float ds18b20_tempc;

    public:
        LPGauge(uint8_t id) :
            LoraNode(USER_LED, id),
            ds(DS18B20_D0)
        {
            pinMode(AH49E_GND, OUTPUT);
            digitalWrite(AH49E_GND, HIGH);
            pinMode(LIPO_CHARGER_EN, OUTPUT);
            digitalWrite(LIPO_CHARGER_EN, LOW);
            pinMode(DS18B20_PWR, OUTPUT);
            digitalWrite(DS18B20_PWR, LOW);
        }

        void send_update(uint8_t dest)
        {
            String msg;
            msg += String(",fvb:") + String(vbat);
            msg += String(",hv:") + String(ah49e_out);
            msg += String(",t1:") + String(ds18b20_tempc);
            msg += String(",rssi:") + String(rf95.lastRssi());
            msg += String(",snr:") + String(rf95.lastSNR());
            msg += String(",txpwr:") + String(txpwr);
            if (tx_rtt > 0)
                msg += String(",rtt:") + String(tx_rtt);
            send_msg(dest, msg);
        }

        virtual void run()
        {
            led_ping(1);

            digitalWrite(AH49E_GND, LOW);
            delay(2);
            ah49e_out = analogRead(AH49E_OUT) * 3.3 / 1024;
            digitalWrite(AH49E_GND, HIGH);
/*
            digitalWrite(DS18B20_PWR, HIGH);
            byte addr[8] = {0x28, 0xC6, 0xE1, 0x76, 0xE0, 0x01, 0x3C, 0x9B};
            start_18B20(ds, addr);
            deep_sleep(0.800);     // maybe 750ms is enough, maybe not
            ds18b20_tempc = read_18B20(ds, addr);
            digitalWrite(DS18B20_PWR, LOW);

            digitalWrite(LIPO_CHARGER_EN, HIGH);
            delay(2);
            vbat = analogRead(VBAT) * 4.244 / 1024;
            digitalWrite(LIPO_CHARGER_EN, LOW);
*/
            send_update(0);

            uint8_t rf95_buf[RH_RF95_MAX_MESSAGE_LEN];
            uint8_t len = sizeof(rf95_buf);
            uint8_t from;

            if (manager.available())
            {
                if (manager.recvfromAck(rf95_buf, &len, &from))
                {
                    rf95_buf[len] = '\0';
                    String msg((char*)rf95_buf);
                    Serial.println(" Rx: " + msg);
                }
            }
            led_ping(3);
            rf95.sleep();
        }
};

LPGauge *node;

void setup()
{
    Serial.begin(115200);
    //while (!Serial) delay(10);
    node = new LPGauge(3);
}


void loop()
{
    node->run();
    deep_sleep(300);
}
