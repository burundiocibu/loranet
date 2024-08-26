#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <SPI.h>

#include <MacroLogger.h>
#include "wifi_setup.hpp"
#include "renogyrover.hpp"
#include "PeriodicTimer.hpp"
#include "LinearActuator.hpp"
#include "Gate.hpp"
#include "LoraNode.hpp"
#include "secrets.hpp"

// This hardware is a Heltec wifi-lora esp32 v3 module

// SSD1306 OLED display
#define SSD1306_SCL 18
#define SSD1306_SDA 17
#define SSD1306_RST 21

// SX1262 LoRa radio
#define SX1262_NSS 8
#define SX1262_RST 12
#define SX1262_BUSY 13
#define SX1262_DIO1 14

#define NODE_ADDRESS 2

#define GATE_LOCK 41
#define POE_ENABLE 42
#define REMOTE_RECEIVER 39

#define MD10C_PWM_PIN 46
#define MD10C_DIR_PIN 45
#define MD10C_PWM_CHAN 0

#define ENCODER_PULSE_PIN 6
#define ENCODER_LIMIT_PIN 7

#define RENOGY_RXD 47
#define RENOGY_TXD 33

#define USER_BUTTON1 0

MD10C* motor;
LinearActuator* actuator;
Gate* gate;
RenogyRover* scc;
LoraNode* node;
U8G2_SSD1306_128X64_NONAME_1_HW_I2C* display;
const char* hostname = "driveway-gate";

void setup()
{
    pinMode(MD10C_PWM_PIN, OUTPUT);
    digitalWrite(MD10C_PWM_PIN, 0);

    // Console
    Serial.begin(115200);
    delay(5000);
    Logger::set_level(Logger::Level::TRACE);

    pinMode(POE_ENABLE, OUTPUT);
    digitalWrite(POE_ENABLE, 1);

    pinMode(USER_BUTTON1, INPUT);
    pinMode(REMOTE_RECEIVER, INPUT_PULLUP);

    motor = new MD10C(MD10C_PWM_PIN, MD10C_DIR_PIN, MD10C_PWM_CHAN);
    actuator = new LinearActuator(ENCODER_PULSE_PIN, ENCODER_LIMIT_PIN, motor);
    gate = new Gate(actuator, GATE_LOCK);
    
    Serial1.begin(9600, SERIAL_8N1, RENOGY_RXD, RENOGY_TXD);
    scc = new RenogyRover(Serial1);
    scc->load_on(1);

    node = new LoraNode(SX1262_NSS, SX1262_DIO1, SX1262_RST, SX1262_BUSY, NODE_ADDRESS);

    display = new U8G2_SSD1306_128X64_NONAME_1_HW_I2C(U8G2_R3, SSD1306_RST, SSD1306_SCL, SSD1306_SDA);
    display->begin();
    display->setFont(u8g2_font_6x13_me);
    display->firstPage();
    do {
        display->setCursor(0, 12);
        display->print(hostname);
    } while ( display->nextPage() );

    wifi_setup(hostname);
}
