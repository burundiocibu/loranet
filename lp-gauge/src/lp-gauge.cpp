// -*- coding: utf-8 -*-
// This is for a DIYmall LoRa32u4 board; appears quite similiar to the Adafruit 32u4 lora feather
// White user led on pin 13
// Orange charging status led
// Must drive PB0 high to enable lipo charger
#include <SPI.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>

// http://www.airspayce.com/mikem/arduino/RadioHead/
// http://www.airspayce.com/mikem/arduino/RadioHead/classRH__RF95.html
#include <RHReliableDatagram.h>
#include <RH_RF95.h>


// for feather32u4
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7 // This is from the pinout image, not the text

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

#define NODE_ADDRESS 3
RHReliableDatagram manager(rf95, NODE_ADDRESS);

#define FEATHER_VBAT 9  // connected to feather Vbatt through a 1/2 divider network
#define LIPO_CHARGER_EN PB0

#define HALL_SENSOR_PWR 11
#define HALL_SENSOR 10

#define USER_LED 13


// 5 to 23 dB on this device
int txpwr = 13;

int tx_rtt = 0;


// watchdog interrupt
ISR (WDT_vect)
{
    wdt_disable();
}

void deep_sleep(unsigned sec)
{
    for (unsigned naps = sec/4; naps; naps--)
    {    // clear various "reset" flags
        MCUSR = 0;
        // allow changes, disable reset
        WDTCSR = bit (WDCE) | bit (WDE);
        // set interrupt mode and an interval
        WDTCSR = bit (WDIE) | bit (WDP3);    // set WDIE, and 4 seconds delay
        wdt_reset();  // pat the dog

        set_sleep_mode (SLEEP_MODE_PWR_DOWN);
        sleep_enable();
        sleep_cpu();
        sleep_disable();
    }
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


void send_update(uint8_t dest)
{
    float feather_vbat = analogRead(FEATHER_VBAT) * 4.244 / 1024;
    float hall_voltage = analogRead(HALL_SENSOR) * 3.3 / 1024;

    String msg;
    msg += String(",fvb:") + String(feather_vbat);
    msg += String(",hv:") + String(hall_voltage);
    msg += String(",rssi:") + String(rf95.lastRssi());
    msg += String(",snr:") + String(rf95.lastSNR());
    msg += String(",txpwr:") + String(txpwr);
    if (tx_rtt > 0)
        msg += String(",rtt:") + String(tx_rtt);
    send_msg(dest, msg);
}


class LoraNode
{
    private:
        const byte ledPin;  // LED to activate when running
        const unsigned long interval; // delay between waking up, in ms

    public:
        LoraNode(byte ledPin, int interval) : ledPin(ledPin), interval(interval)
        {
            while (!Serial) delay(10);
            led_ping(15);

            pinMode(ledPin, OUTPUT);
            led_ping(1);
            pinMode(RFM95_RST, OUTPUT);
            digitalWrite(RFM95_RST, HIGH);

            pinMode(LIPO_CHARGER_EN, OUTPUT);
            digitalWrite(LIPO_CHARGER_EN, HIGH);

            pinMode(HALL_SENSOR_PWR, OUTPUT);

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

            if (!rf95.setFrequency(RF95_FREQ))
            {
                Serial.println("setFrequency failed");
                while (1);
            }
            Serial.print("Freq set to ");
            Serial.println(RF95_FREQ);

            rf95.setTxPower(txpwr, false);
        }

        void led_ping(unsigned ms)
        {
            digitalWrite(ledPin, HIGH);
            delay(ms);
            digitalWrite(ledPin, LOW);
        }

        virtual void run()
        {
            led_ping(1);

            digitalWrite(HALL_SENSOR_PWR, HIGH);
            send_update(0);
            digitalWrite(HALL_SENSOR_PWR, LOW);
            rf95.sleep();

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
            deep_sleep(interval);
        }
};


class LPGauge : public LoraNode
{
    LPGauge() : LoraNode(USER_LED, 12)
    {
    }
};

LoraNode *node;

void setup()
{
    Serial.begin(115200);
    node = new LoraNode(USER_LED, 5000);
}


void loop()
{
    node->run();
}
