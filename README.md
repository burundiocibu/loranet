## LoRaNet bridge to MQTT

Python code to talk to a Adafruit LoRa feather via a LoRa bonnet attached to a raspberry pi.

platform-io is the dev env for the Adafruit feather (avr 32u4 + )



## Not Inventing it here


Ideas on making a LoRa <-> MQTT gateway and LoRa devices to deploy

https://heltec-automation-docs.readthedocs.io/en/latest/general/subscribe_mqtt_messages.html

or 

https://github.com/1technophile/OpenMQTTGateway

Lookls like the above is not very capable; it simply forwards everything; doesn't look like it has the ability to accept a more complex interface
like home assistant entities would want to see.

or

https://www.mysensors.org/build/raspberry

^ this would work. I don't like it

Just do my own mqtt 'client'

## LoRaNet 

### LoRa <-> MQTT protocol


S   Status request
S,p=v,[p=v,...] Status response
C: close gate
O: open gate
Pnnn: set power to nnn

Parameters (p in status response):
gpos: gate position, int,  0-100
gvb:  gate battery voltage, float, 0..15
gc:   gate closed, bool, 0|1
fvb:  feather battery voltage, float, 0..5 
rssi: receive signal strength indicator, int, -128..0
snr:  receive signal to noise ratio, float, ??
txpwr: transmit power, int, 3..23

### bridge.py

classes:
    LoRaBase : Interface to the base station LoRa radio; no threads tx/rx calls can take hundreads of ms
    DeviceConfig: data to describe a home assistant/mqtt device
    EntityConfig: data to describe a home assistant/mqtt entity (a device can have multiple entities)
    EntityBase: base class for the functionality of a single entity
    LoRaNode: 

define callbacks for objects that need