// -*- coding: utf-8 -*-

#include <LoraNode.hpp>
#include <OneWireWrapper.hpp>

#define DISABLE_SLEEP
#include <DeepSleep.hpp>

// 32u4 i/o assignments
#define DS18B20_PWR 6
#define VBAT 9  // connected to Vbatt through a divider network
#define DS18B20_D0 12
#define USER_LED 13
#define LIPO_CHARGER_EN PB0 // must be brough high to enable measuring or charging the battery


class DevNode : public LoraNode
{
    private:
        OneWire ds;
        float vbat;
        float ds18b20_tempc;
        byte ds18b20_addr[8] = {0,0,0,0,0,0,0,0};

    public:
        DevNode(uint8_t id) :
            LoraNode(USER_LED, id),
            ds(DS18B20_D0)
        {
            pinMode(LIPO_CHARGER_EN, OUTPUT);
            digitalWrite(LIPO_CHARGER_EN, LOW);
            pinMode(DS18B20_PWR, OUTPUT);
            digitalWrite(DS18B20_PWR, LOW);

        }

        void send_update(uint8_t dest)
        {
            String msg;
            msg += String(",fvb:") + String(vbat);
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

            digitalWrite(DS18B20_PWR, HIGH);
            // This will find all devices but use the last found
            if (ds18b20_addr[0] == 0)
            {
                deep_sleep(0.1);
                scan_bus(ds, ds18b20_addr);
            }
            start_18B20(ds, ds18b20_addr);
            deep_sleep(0.800);
            ds18b20_tempc = read_18B20(ds, ds18b20_addr);
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

DevNode *node;

void setup()
{
    Serial.begin(115200);
    while (!Serial) delay(10);
    node = new DevNode(4);
}


void loop()
{
    node->run();
    deep_sleep(5);
}
