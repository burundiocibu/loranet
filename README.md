# LoRaNet: a private network of LoRa nodes using the RadioHead library
This is for a private network of LoRa nodes communicating to a single gateway which can plumb the entites into homeassistant compatable mqtt entities and export
prometheus endpoints for datalogging.

# Bridge
## LoRaNet bridge
Python code to talk to a LoRa devices via a LoRa bonnet attached to a raspberry pi.
Note that this code is not intended to be a python package but run as a systemd service.

## LoRaNet nodes
platform-io is the dev env for the Adafruit LoRa feather (avr 32u4 + )



## Not Inventing it here
Ideas on making a LoRa <-> MQTT gateway and LoRa devices to deploy

TTN: I don't have coverage here and don't want to put up a gateway.
Also, I don't think I could get sub second responsiveness to commands.

https://heltec-automation-docs.readthedocs.io/en/latest/general/subscribe_mqtt_messages.html

or

https://github.com/1technophile/OpenMQTTGateway

Lookls like the above is not very capable; it simply forwards everything; doesn't look like it has the ability to accept a more complex interface like home assistant entities would want to see.

or

https://www.mysensors.org/build/raspberry

^ this would work. I don't like it

Just do my own mqtt 'client'
