lp-gauge.md
# lp-gauge

## Hardware
diymall avr 32u4 processor with a SX1276 like lora radio
18650 4.2V LiPo power cell connected to the jst battery

## Power

Using the prosenb/DeepSleepScheduler and sleeping for 5 seconds between readings/tx
about 5 mA use with calling rmf95.sleep() before going to sleep

When running & transmitting at 23 dB, current draw is about 140 mA

That was while forgetting to shut off ah49e during sleep.

With the ah49e powered off, I can't measure the draw with a 1ohm resistor and the scope.

Entire wake, measure, tx, rx ack, sleep cycle takes about 950ms

From looking at the current draw, the rx of the ack takes around 50 ms, the tx takes about 125 ms

Wake to tx seems to take 700 ms!
I think this is the rxfromack call...


## Sensors
### AH49E
generic AH49E hall-effect sensor connected to 1/8" TRS plug
https://www.diodes.com/assets/Datasheets/AH49E.pdf

AH49E
pin    function   TRS
1      Vcc        ring
2      Gnd        shield
3      out        tip

This is potted in an RD2 cap from the LP tank's mechanical gauge with a 1m cable to the trs plug.

### DS18B20
https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf
connected to a 1/8" TRS plug

18B20
Pin          TRS
1    Gnd     shield
2    Dq      tip
3    Vdd     ring
