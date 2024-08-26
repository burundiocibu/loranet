// -*- coding: utf-8 -*-
#include "lp_gauge.hpp"
#include "utils.hpp"

void loop()
{
    digitalWrite(AH49E_PWR, HIGH);
    delay(2);
    float ah49e_out = analogRead(AH49E_OUT) * 3.3 / 1024;
    digitalWrite(AH49E_PWR, LOW);

    digitalWrite(DS18B20_PWR, HIGH);
    byte addr[8] = {0x28, 0xC6, 0xE1, 0x76, 0xE0, 0x01, 0x3C, 0x9B};
    start_18B20(ds, addr);
    deep_sleep(0.800);     // maybe 750ms is enough, maybe not
    float ds18b20_tempc = read_18B20(ds, addr);
    digitalWrite(DS18B20_PWR, LOW);

    digitalWrite(LIPO_CHARGER_EN, HIGH);
    delay(2);
    float vbat = analogRead(VBAT) * 4.244 / 1024;
    digitalWrite(LIPO_CHARGER_EN, LOW);


    String msg = 
        "ut:" + String(int(uptime())) +
        ",vb:" + String(vbat,2) +
        ",fvb:" + String(vbat) +
        ",hv:" + String(ah49e_out) +
        ",t1:" + String(ds18b20_tempc);

    int tx_count = 0;
    while (tx_count < 2)
    {
        Serial.print("tx " + String(tx_count) + ":");
        Serial.println(msg);
        rf95.send(msg.c_str(), msg.length());
        rf95.waitPacketSent();
        tx_count++;

        if (rf95.waitAvailableTimeout(500))
        {
            uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
            uint8_t len = sizeof(buf);
            if (rf95.recv(buf, &len))
            {
                Serial.print("got reply: ");
                Serial.print((char *)buf);
                Serial.print(", RSSI: ");
                Serial.println(rf95.lastRssi(), DEC);
            }
            break;
        }
        else
            Serial.println("Rx Timeout");
    }
    rf95.sleep();
    Serial.println("sleeping");
    delay(50);
    deep_sleep(10);
}
