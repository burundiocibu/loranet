# LoRaNet: a private network of LoRa nodes using the RadioHead library
This is for a private network of LoRa nodes communicating to a single gateway which can plumb the entites into homeassistant compatable
mqtt entities.

## LoRaNet bridge
Python code to talk to a Adafruit LoRa feather via a LoRa bonnet attached to a raspberry pi.
development is done in a python venv virtual environment (loranet/bridge/.venv)

### Classes
    LoRaBase : Interface to the base station LoRa radio; no threads tx/rx calls can take hundreads of ms
    DeviceConfig: data to describe a home assistant/mqtt device
    EntityConfig: data to describe a home assistant/mqtt entity (a device can have multiple entities)
    EntityBase: base class for the functionality of a single entity
    LoRaNode: 

define callbacks for objects that need

### systemd unit
A user systemd unit is defined in
`loranet/bridge/loranet-bridge.service`

Install it doing the following:
```bash
mkdir -p ~/.config/systemd/user
cp loranet/bridge/loranet-bridge.service ~/.config/systemd/user
loginctl enable-linger $USER
systemctl --user enable thin@redmine



## LoRaNet nodes
platform-io is the dev env for the Adafruit LoRa feather (avr 32u4 + )




## Not Inventing it here
Ideas on making a LoRa <-> MQTT gateway and LoRa devices to deploy

https://heltec-automation-docs.readthedocs.io/en/latest/general/subscribe_mqtt_messages.html

or 

https://github.com/1technophile/OpenMQTTGateway

Lookls like the above is not very capable; it simply forwards everything; doesn't look like it has the ability to accept a more complex interface like home assistant entities would want to see.

or

https://www.mysensors.org/build/raspberry

^ this would work. I don't like it

Just do my own mqtt 'client'
