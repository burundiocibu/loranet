// -*- coding: utf-8 -*-

#include <SPI.h>

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

#define NODE_ADDRESS 2
RHReliableDatagram manager(rf95, NODE_ADDRESS);

#define FEATHER_VBAT 9  // connected to feather Vbatt through a 1/2 divider network

#define GATE_SAFE 3 // GPIO output to relay
#define GATE_OPEN 2 // GPIO output to relay

#define GATE_VBAT 12 // A11 (w dividerr)
#define GATE_POS 10  // A10 (may not work)
#define GATE_CLOSED 11  // magnetic 

// 5 to 23 dB on this device
int txpwr = 13;

void setup() 
{
    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);

    pinMode(GATE_OPEN, OUTPUT); 
    pinMode(GATE_SAFE, OUTPUT);
    pinMode(GATE_CLOSED, INPUT);

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
    // This is the 
    float feather_vbat = analogRead(FEATHER_VBAT) * 2 * 3.3 / 1024;
    float gate_vbat = analogRead(GATE_VBAT) * 3.3/1024;
    float gate_pos = analogRead(GATE_POS) * 3.3/1024;
    uint8_t gate_closed = digitalRead(GATE_CLOSED);

    String msg;
    msg += String("gpos:") + String(gate_pos);
    msg += String(",gvb:") + String(gate_vbat);
    msg += String(",gc:") + String(gate_closed);
    msg += String(",fvb:") + String(feather_vbat);
    msg += String(",gvb:") + String(gate_vbat);
    msg += String(",rssi:") + String(rf95.lastRssi());
    msg += String(",snr:") + String(rf95.lastSNR());
    msg += String(",txpwr:") + String(txpwr);
    Serial.println(msg);

    if (!manager.sendtoWait((uint8_t*)msg.c_str(), msg.length(), from))
        Serial.println("sendtoWait failed");
}


void open_gate()
{
    digitalWrite(GATE_OPEN, 1);
    delay(100);
    digitalWrite(GATE_SAFE, 1);
    delay(10);
    digitalWrite(GATE_OPEN, 0);
}


void close_gate()
{
    digitalWrite(GATE_OPEN, 0);
    delay(10);
    digitalWrite(GATE_SAFE, 0);
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
                case 'O':
                    open_gate();
                    break;

                case 'C':
                    close_gate();
                    break;

                case 'S':
                    delay(10); // give the receiver a chance to start listening
                    break;

                case 'P':
                    int pwr = atoi((char*)&rf95_buf[2]);
                    if (pwr >= 3 && pwr <= 23)
                        txpwr = pwr;
                    delay(10); // give the receiver a chance to start listening
                    break;
            }

            send_status(from);
        }
        else
            Serial.println("recvfromAck error");
    }
}
