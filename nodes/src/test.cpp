// -*- coding: utf-8 -*-
#include "PeriodicTimer.hpp"
#include "LinearActuator.hpp"
#include "utils.hpp"

#define MD10C_PWM_PIN 5
#define MD10C_DIR_PIN 10

#define ENCODER_PIN 2  // Must be on an external interrupt.
#define ENCODER_LIMIT_PIN A0


void setup()
{
    // Console
    Serial.begin(115200);
    while (!Serial) delay(10);
    Serial.println("Node test");
}


void loop()
{
    static MD10C motor(MD10C_PWM_PIN, MD10C_DIR_PIN);
    static LinearActuator gate(ENCODER_PIN, ENCODER_LIMIT_PIN, &motor);

    static PeriodicTimer update_timer(1000);
    if (update_timer.time())
        Serial.println(runtime() + gate.get_status());
}

