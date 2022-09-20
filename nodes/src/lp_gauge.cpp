// -*- coding: utf-8 -*-

#include <LoraNode.hpp>
#include <OneWireWrapper.hpp>

//#define DISABLE_SLEEP
#include <DeepSleep.hpp>

// 32u4 i/o assignments
#define DS18B20_PWR 6
#define VBAT 9  // connected to Vbatt through a divider network
#define AH49E_OUT 10
#define AH49E_PWR 11
#define DS18B20_D0 12
#define USER_LED 13
#define LIPO_CHARGER_EN PB0 // must be brough high to enable measuring or charging the battery


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
            pinMode(AH49E_PWR, OUTPUT);
            digitalWrite(AH49E_PWR, LOW);
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

            digitalWrite(AH49E_PWR, HIGH);
            delay(2);
            ah49e_out = analogRead(AH49E_OUT) * 3.3 / 1024;
            digitalWrite(AH49E_PWR, LOW);

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
