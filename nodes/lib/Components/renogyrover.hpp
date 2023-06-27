// -*- coding: utf-8 -*-

// https://github.com/4-20ma/ModbusMaster
#include <ModbusMaster.h>

#ifndef RENOGY_SCC_HPP
#define RENOGY_SCC_HPP

// The modbus ICD for the renogy rover/wander series
// https://docs.google.com/document/d/1OSW3gluYNK8d_gSz4Bk89LMQ4ZrzjQY6/edit

// chaging_state:
// 0 deactivated
// 1 activated
// 2 mppt charging
// 3 equalizing charging
// 4 boost charging
// 5 float charging
// 6 current limiting

// controller_fault:
// B0: battery over-discharge
// B1: battery over-voltage
// B2: battery under-voltage
// B3: load short circuit
// B4: load over-current
// B5: controller temp too high
// B6: ambient temp too high
// B7: PV input overpower
// B8: PV input short circuit
// B9: PV input over-voltage
// B10: PV counter-current
// B11 PV working point over-voltage
// B12: PV reversely connected
// B13: anti-reverse MOS short
// B14: charge MOS short circuit
// B15: reserved


class RenogyRover
{
    public:
        RenogyRover(Stream &serial, uint8_t load=1);

        float battery_percentage() {return read_register(0x0100); };
        float battery_voltage() {return 0.1  * read_register(0x0101); };  // Volts
        float battery_current() {return 0.01 * read_register(0x0102); }; // Amps delivered to battery, zero for drain?
        float battery_temperature() {return 1.0 * int8_t(read_register(0x0103) & 0xff); }; // centigrade
        float controller_temperature() {return 1.0 * int8_t((read_register(259)>>8) & 0xff); }; // centigrade
        float load_voltage() {return 0.1 * read_register(0x0104); }; // volts
        float load_current() {return 0.01 * read_register(0x0105); }; // amps
        float load_power() {return 1.0 * read_register(0x0106); }; // W
        float solar_voltage() {return 0.1 * read_register(0x107); }; // Volts
        float solar_current() {return 0.01 * read_register(0x108); }; //Amps
        float charging_power() {return 1.0 * read_register(0x0109); }; // W
        uint8_t load_on(bool v) {return node.writeSingleRegister(0x010a, v?1:0); };
        float charging_ah_today() {return read_register(0x0111); };
        float discharging_ah_today() {return read_register(0x0112); };
        int charging_state() {return read_register(0x0120) & 0xff; };
        uint8_t load_on() {return read_register(0x120) >> 15 ; };
        uint16_t controller_fault() {return read_register(0x0121); };
        uint16_t battery_type() {return read_register(0xe004); };
        float discharging_limit_voltage() {return 0.1 * read_register(0xe00e); };

        String status();

    private:
        ModbusMaster node;
        uint16_t read_register(uint16_t reg);
};
#endif