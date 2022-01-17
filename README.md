# lora-radio

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

Messages to send be a cover:
homeassistant/cover/pico1/test_cover/config
{
    "dev_cla":"gate",
    "pos_t":"pico1/cover/test_cover/position/state"
    "set_pos_t":"pico1/cover/test_cover/position/command"
    "name":"Test Cover"
    "cmd_t":"pico1/cover/test_cover/command"
    "avty_t":"pico1/status"
    "uniq_id":"ESPcovertest_cover"
    "dev":{"ids":"5002918689c4"
    "name":"pico1"
    "sw":"esphome v2021.12.3 Jan 17 2022 14:49:35" "mdl":"tinypico" "mf":"espressif"}
}
Must receive:
pico1/cover/test_cover/command
OPEN
or
CLOSE

must send:
pico1/status
online

pico1/cover/test_cover/state
closed|open

pico1/cover/test_cover/position/state
0..100