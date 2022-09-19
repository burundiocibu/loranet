# lp-gauge

## Hardware
diymall avr 32u4 processor with a SX1276 like lora radio
18650 4.2V LiPo power cell connected to the jst battery

## Power
From looking at the power draw on the scope, it takes the 32u4 2 ms to wake from sleep.
Current draw during this period is approx 4 ma; its hard to measrure that.
4.1 V * .0004 A * 0.002 s = 3.3 uJ

a 2 Ah LiPo cell has 3.7V * 2 Ah 3600 = 26.7 kJ

waking every second and doing nothing else, this cell would last 255 years


## Sensors
### AH49E
generic AH49E hall-effect sensor connected to 1/8" TRS plug
https://www.diodes.com/assets/Datasheets/AH49E.pdf

AH49E
pin    function   TRS
1      Vcc        ring
2      Gnd        shield
3      out        tip

The Vcc is connected to the Vbat pin on the board
The Gnd is connected to pin 11 of the 32u4 and is brough high while sleeping to reduce energy usage

This is potted in an RD2 cap from the LP tank's mechanical gauge with a 1m cable to the trs plug.

### DS18B20
https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf
connected to a 1/8" TRS plug

18B20
Pin          TRS
1    Gnd     shield
2    Dq      tip
3    Vdd     ring

Vdd on this is connected to pin 6 on the 32u4 and is driven low while sleeping to reduce energy usage
