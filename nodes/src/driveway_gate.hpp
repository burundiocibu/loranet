// -*- coding: utf-8 -*-

#include <Arduino.h>
#include "renogyrover.hpp"
#include "PeriodicTimer.hpp"
#include "LinearActuator.hpp"
#include "utils.hpp"

#define RFM95_RST 4
#define RFM95_INT 7
#define RFM95_CS 8
#define RF95_FREQ 915.0

// OLED display
#define SSD1306_SCL 18
#define SSD1306_SDA 17
#define SSD1306_RST 21

/*
SX1262 I/O
GPIO8  | LoRa_NSS
GPIO9  | LoRa_SCK
GPIO10 | LoRa_MOSI
GPIO11 | LoRa_MISO
GPIO12 | LoRa_RST
GPIO13 | LoRa_BUSY

e11 | J2.11 | gpio34 | TTL-RxD
e12 | J2.12 | gpio33 | TTL-TxD
*/

#define NODE_ADDRESS 2

#define VBAT 34 // 1  // connected to Vbatt through a 1/2 divider network

#define GATE_LOCK 2
#define DRIVEWAY_RECEIVER 39
#define REMOTE_RECEIVER 38

#define MD10C_PWM_PIN 22 // 42
#define MD10C_DIR_PIN 21 // 41
#define MD10C_PWM_CHAN 0

#define ENCODER_PULSE_PIN 36 // 46
#define ENCODER_LIMIT_PIN 37 //45

#define RENOGY_TXD 27 //34
#define RENOGY_RXD 26 //33

#define USER_BUTTON1 35 // 
#define USER_BUTTON2 0 //

MD10C* motor;
LinearActuator* gate;
RenogyRover* scc;


void setup()
{
    // Console
    Serial.begin(115200);
    while (!Serial) delay(10);
    Serial.println("test_gate_hw ");
    pinMode(VBAT, INPUT);

    pinMode(GATE_LOCK, OUTPUT);
    digitalWrite(GATE_LOCK, LOW);


    pinMode(DRIVEWAY_RECEIVER, INPUT);
    pinMode(REMOTE_RECEIVER, INPUT_PULLUP);

    pinMode(USER_BUTTON1, INPUT);
    pinMode(USER_BUTTON2, INPUT);

    motor = new MD10C(MD10C_PWM_PIN, MD10C_DIR_PIN, MD10C_PWM_CHAN);
    gate = new LinearActuator(ENCODER_PULSE_PIN, ENCODER_LIMIT_PIN, motor);
    
    Serial1.begin(9600, SERIAL_8N1, RENOGY_TXD, RENOGY_RXD);
    scc = new RenogyRover(Serial1);
    
}