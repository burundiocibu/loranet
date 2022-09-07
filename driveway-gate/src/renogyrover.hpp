// -*- coding: utf-8 -*-
// https://github.com/4-20ma/ModbusMaster
#include <ModbusMaster.h>
// chaging_state:
// 0 deactivated
// 1 activated
// 2 mppt charging
// 3 equalizing charging
// 4 boost charging
// 5 float charging
// 6 current limiting

class RenogyRover
{
    public:
        RenogyRover(Stream &serial);

        float battery_percentage() {return read_register(256); };
        float battery_voltage()    {return 0.1  * read_register(257); };  // Volts
        float battery_temperature() {return 1.0 * int8_t(read_register(259) & 0xff); }; // centigrade
        float controller_temperature() {return 1.0 * int8_t((read_register(259)>>8) & 0xff); }; // centigrade
        float load_voltage() {return 0.1 * read_register(260); }; // volts
        float load_current() {return 0.01 * read_register(261); }; // amps
        float load_power() {return 1.0 * read_register(262); }; // W
        float solar_voltage() {return 0.1 * read_register(263); }; // volts
        float solar_current() {return 0.01 * read_register(264); }; //amps
        float solar_power() {return 1.0 * read_register(265); }; // W
        uint8_t load_on(bool v) {return node.writeSingleRegister(266, v?1:0); };
        float charging_ah_today() {return read_register(273); };
        float discharging_ah_today() {return read_register(274); };
        int charging_state() {return read_register(288) & 0xff; };
        uint8_t load_on() {return read_register(288) >> 15 ; };
        float battery_capicity()   {return 0.1  * read_register(57346); };  // Ah

    private:
        ModbusMaster node;
        uint16_t read_register(uint16_t reg);
};