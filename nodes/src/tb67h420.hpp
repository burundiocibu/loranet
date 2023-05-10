// -*- coding: utf-8 -*-

#include <Arduino.h>

// Pololu brushed dc motor driver https://www.pololu.com/product/2999/resources
// TB67H420 motor driver: https://toshiba.semicon-storage.com/info/TB67H420FTG_datasheet_en_20201016.pdf?did=59110&prodName=TB67H420FTG
// Note that this only drives the a second (or for when HBMODE is jumpered high on board)

class TB67H420
{
    public:
        TB67H420(uint8_t pwma, uint8_t ina1, uint8_t ina2, uint8_t lo1, uint8_t lo2);
        void run(int speed);
        void coast() { digitalWrite(ina1, LOW); digitalWrite(ina2, LOW); };
        void stop() { digitalWrite(ina1, HIGH); digitalWrite(ina2, HIGH); };
        int error(); // 1 = load open, 2 = over current, 3 = over thermal
        void clear_error() {coast();};
        void set_pwm_duty(int _pwm_duty) { pwm_duty = min(100, _pwm_duty); OCR3A = uint16_t((pwm_duty/100.0) * pwm_top); };
        int get_pwm_duty() {return pwm_duty;};
        int get_pwm_period() {return pwm_period;};

    private:
        uint8_t pwma, ina1, ina2, lo1, lo2; // pins used to talk to the driver
        int pwm_top;  // max count of OC reg
        int pwm_duty; // duty cycle in percent
        int pwm_period; // 1/frequency of pwm, in ms

};