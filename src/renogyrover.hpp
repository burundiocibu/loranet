// -*- coding: utf-8 -*-
#include <ModbusMaster.h>

class RenogyRover
{
    public:
        RenogyRover(Stream &serial);
        String status_msg();
        
        float battery_voltage()    {return 0.1  * read_register(257); };  // Volts
        float battery_percentage() {return read_register(256); };
        float battery_capicity()   {return 0.1  * read_register(57346); };  // Ah
        float batery_temperature() {return 1.0 * int8_t(read_register(259) & 0xff); }; // centigrade
        float controller_temperature() {return 1.0 * int8_t((read_register(259)>>8) & 0xff); }; // centigrade
        float load_voltage() {return 0.1 * read_register(260); }; // volts
        float load_current() {return 0.01 * read_register(261); }; // amps
        float load_power() {return 1.0 * read_register(262); }; // ?? units
        float solar_voltage() {return 0.1 * read_register(263); }; // volts
        float solar_current() {return 0.01 * read_register(264); }; //amps
        float solar_power()   {return 1.0 * read_register(265); };
        int charging_status() {return read_register(288) & 0xff; };
        float charging_ah_today() {return read_register(273); };
        float discharging_ah_today() {return read_register(274); };

    private:
        ModbusMaster node;
        uint16_t read_register(uint16_t reg);
};