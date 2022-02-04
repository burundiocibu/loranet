## LoRaNet bridge to MQTT

python code to talk to a Adafruit LoRa feather attached to a raspberry pi.


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
