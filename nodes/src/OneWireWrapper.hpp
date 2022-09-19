// -*- coding: utf-8 -*-
// https://registry.platformio.org/libraries/paulstoffregen/OneWire/examples/DS18x20_Temperature/DS18x20_Temperature.ino
#include <OneWire.h>

// Code below was reworked from one of the examples

void start_18B20(OneWire& ds, byte* addr)
{
    ds.reset();
    ds.select(addr);
    ds.write(0x44, 1);        // start conversion, with parasite power on at the end
}


float read_18B20(OneWire& ds, byte* addr)
{
    if (!ds.reset())
        return 0;
    ds.select(addr);
    ds.write(0xBE);         // Read Scratchpad

    byte data[9];
    for (int i = 0; i < 9; i++)
        data[i] = ds.read();

    // Convert the data to actual temperature
    // because the result is a 16 bit signed integer, it should
    // be stored to an "int16_t" type, which is always 16 bits
    // even when compiled on a 32 bit processor.
    int16_t raw = (data[1] << 8) | data[0];
    if (addr[0] == 0x10)
    {
        // ds1820, ds18s20
        raw = raw << 3; // 9 bit resolution default

        // "count remain" gives full 12 bit resolution
        if (data[7] == 0x10)
            raw = (raw & 0xFFF0) + 12 - data[6];
    }
    else
    {
        // ds18b20
        byte cfg = (data[4] & 0x60);
        // at lower res, the low bits are undefined, so let's zero them
        if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
        else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
        else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
        //// default is 12 bit resolution, 750 ms conversion time
    }

    return (float)raw / 16.0;
}


void scan_bus(OneWire ds)
{
    byte i;
    byte addr[8];

    while (ds.search(addr))
    {
        Serial.print("ROM =");
        for( i = 0; i < 8; i++)
        {
            Serial.write(' ');
            Serial.print(addr[i], HEX);
        }

        if (OneWire::crc8(addr, 7) != addr[7])
        {
            Serial.println("CRC is not valid!");
            continue;
        }
        Serial.println();

        // the first ROM byte indicates which chip
        switch (addr[0])
        {
            case 0x10:
                Serial.println("  Chip = DS18S20");  // or old DS1820
                break;
            case 0x28:
                Serial.println("  Chip = DS18B20");
                break;
            case 0x22:
                Serial.println("  Chip = DS1822");
                break;
            default:
                Serial.println("Device is not a DS18x20 family device.");
                continue;
        }
        start_18B20(ds, addr);
        delay(1000);
        float temp_c = read_18B20(ds, addr);
        Serial.print("Temperature=");
        Serial.println(temp_c);
    }

    Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    return;
}
