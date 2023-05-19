// -*- coding: utf-8 -*-

#include "md10c.hpp"
#include "utils.hpp"

#define MD10C_PWM 5
#define MD10C_DIR 10
MD10C motor(MD10C_PWM, MD10C_DIR);


void setup()
{
    // Console
    Serial.begin(115200);
    while (!Serial) delay(10);
    Serial.println("Node test");
    motor.run(10);
}


void loop()
{
    static unsigned long last_motor_update = 0;
    const long motor_update_rate = 2000;
    if (if_dt(last_motor_update, motor_update_rate))
    {
        static int speed=100;
        speed = -speed;
        motor.run(speed);
        Serial.print(runtime() + " speed:");  Serial.println(motor.get_speed());
    }

    static unsigned long last_update = 0;
    if (if_dt(last_update, 5000))
    {
        Serial.print(runtime() + " millis:"); Serial.println(millis());
    }
}

