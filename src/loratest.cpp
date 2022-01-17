#include <SPI.h>

// http://www.airspayce.com/mikem/arduino/RadioHead/
// http://www.airspayce.com/mikem/arduino/RadioHead/classRH__RF95.html
#include <RHReliableDatagram.h>
#include <RH_RF95.h>

// for feather32u4 
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7 // docs say this should be 3 but that doesn't work

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

#define NODE_ADDRESS 2
RHReliableDatagram manager(rf95, NODE_ADDRESS);

#define VBAT_PIN 9       // connected to Vbatt through a 1/2 divider network
#define RELAY_SAFE_PIN 2 // output to relay
#define RELAY_EXIT_PIN 3 // output to relay

// 5 to 23 dB on this device
int txpwr = 5;

void setup() 
{
    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);

    Serial.begin(115200);
    //while (!Serial)
    //    delay(1);
    Serial.println("Feather LoRa TX Test!");

    if (!manager.init())
        Serial.println("manager init failed");

    // manual reset
    if (false)
    {
        digitalWrite(RFM95_RST, LOW);
        delay(10);
        digitalWrite(RFM95_RST, HIGH);
        delay(10);
    }

    // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
    while (!rf95.init())
    {
        Serial.println("LoRa radio init failed");
        while (1);
    }
    Serial.println("LoRa radio init OK!");

    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
    if (!rf95.setFrequency(RF95_FREQ))
    {
        Serial.println("setFrequency failed");
        while (1);
    }
    Serial.print("Set Freq to: ");
    Serial.println(RF95_FREQ);
  
    rf95.setTxPower(txpwr, false);
}

void send_status(uint8_t from)
{
    rf95.setTxPower(txpwr);
    float vbat = analogRead(VBAT_PIN) * 2 * 3.3 / 1024;

    String msg;
    msg += String("vbat:") + String(vbat);
    msg += String(",rssi:") + String(rf95.lastRssi());
    msg += String(",snr:") + String(rf95.lastSNR());
    msg += String(",txpwr:") + String(txpwr);
    Serial.println(msg);

    if (!manager.sendtoWait((uint8_t*)msg.c_str(), msg.length(), from))
        Serial.println("sendtoWait failed");

}


void loop()
{
    if (manager.available())
    {
        uint8_t rf95_buf[RH_RF95_MAX_MESSAGE_LEN];
        uint8_t len = sizeof(rf95_buf);
        uint8_t from;

        if (manager.recvfromAck(rf95_buf, &len, &from))
        {
            rf95_buf[len] = '\0';
            Serial.println((char*)rf95_buf);
            switch (rf95_buf[0])
            {
                case 'S':
                    delay(10); // give the receiver a chance to start listening
                    send_status(from);
                    break;

                case 'P':
                    int pwr = atoi((char*)&rf95_buf[2]);
                    if (pwr >= 3 && pwr <= 23)
                        txpwr = pwr;
                    break;
            }

        }
        else
            Serial.println("recvfromAck error");
    }
}
