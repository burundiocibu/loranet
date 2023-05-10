// -*- coding: utf-8 -*-

#include "tb67h420.hpp"
#include "utils.hpp"

#define TB67_PWMA 5
#define TB67_INA1 10
#define TB67_INA2 11
#define TB67_LO1 6
#define TB67_LO2 3
TB67H420 tb67(TB67_PWMA, TB67_INA1, TB67_INA2, TB67_LO1, TB67_LO2);


void setup()
{
    // Console
    Serial.begin(115200);
    while (!Serial) delay(10);
    Serial.println("LoRaNet test");
}


void loop()
{
    static unsigned long last_motor_update = 0;
    const long motor_update_rate = tb67.get_pwm_period() * 100;
    if (if_dt(last_motor_update, motor_update_rate))
    {
        static int delta=5;
        static int dir = 1;
        int pwm = tb67.get_pwm_duty();
        if (pwm <= 0)
            dir = 1;
        else if (pwm >= 100)
            dir = -1;
        tb67.set_pwm_duty(pwm + delta*dir);
        Serial.print(runtime() + " pwm_duty:"); Serial.println(tb67.get_pwm_duty());
    }

    static unsigned long last_update = 0;
    if (if_dt(last_update, 5000))
    {
        Serial.print(runtime() + " millis:"); Serial.println(millis());
    }
}

