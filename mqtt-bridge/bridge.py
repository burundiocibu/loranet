#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from audioop import add
from copyreg import add_extension
from http.client import GATEWAY_TIMEOUT
import os
import platform
import json
from socket import MsgFlag
import hashlib
import datetime

import paho.mqtt.client as mqtt

import time
import busio
from digitalio import DigitalInOut, Direction, Pull
import board
import adafruit_ssd1306
import adafruit_rfm9x


# Create the I2C interface.
i2c = busio.I2C(board.SCL, board.SDA)

# 128x32 OLED Display
reset_pin = DigitalInOut(board.D4)
display = adafruit_ssd1306.SSD1306_I2C(128, 32, i2c, reset=reset_pin)
# Clear the display.
display.fill(0)
display.text('LoRa mqtt bridge', 0, 0, 1)
display.show()

# Configure LoRa Radio, https://circuitpython.readthedocs.io/projects/rfm9x/en/latest/
# https://github.com/adafruit/Adafruit_CircuitPython_RFM9x
CS = DigitalInOut(board.CE1)
RESET = DigitalInOut(board.D25)
spi = busio.SPI(board.SCK, MOSI=board.MOSI, MISO=board.MISO)
rfm9x = adafruit_rfm9x.RFM9x(spi, CS, RESET, 915.0) # 915 Mhz
tx_power = 23
rfm9x.tx_power = tx_power


rfm9x.destination = 2 #  default is 255 (broadcast) 
rfm9x.node = 1 # RH calls this source, default is 255 (broadcast)
rfm9x.flags = 0 # default is 0


tx_stats = {'count':0, 'err':0 }
rx_stats = {'count':0, 'err':0 }
node = {}


def cname(string:str) -> str:
    return string.replace(" ","-").lower()


def print_stats(msg:str) -> None:
    #print(f"{msg}  tx:{tx_stats}  rx:{rx_stats}  node:{node}")
    pass


def lora_tx(msg:str) -> bool:
    t0 = time.perf_counter()
    tx_stats['count'] += 1
    if not rfm9x.send_with_ack(bytes(msg, "utf-8")):
        tx_stats['err'] += 1
        print_stats("noack")
        return False
    tx_stats['rtt'] = round(1000 * (time.perf_counter() - t0), 1)
    tx_stats['rssi'] = rfm9x.last_rssi
    tx_stats['snr'] = rfm9x.last_snr
    tx_stats['txpwr'] = rfm9x.tx_power
    if True or tx_stats['count'] % 10 == 0:
        print_stats("ok")
    return True
    

def lora_rx(timeout:float = 0.5):
    t0 = time.perf_counter()
    rfm9x.receive_timeout = timeout # seconds
    packet = rfm9x.receive(with_ack=True)
    rx_stats['rtt'] = round(1000 * (time.perf_counter() - t0),)
    if packet is None:
        rx_stats['err'] += 1
        print_stats('timeout')
    else:
        rx_stats['count'] += 1
    return packet


def decode_msg(msg:str) -> dict:
    d = {}
    for s in msg.decode().split(","):
        if len(s):
            n,v = s.split(':')
            d[n] = v
    return d


def on_disconnect():
    print("Disconnecting...")


class DeviceConfig:
    def __init__(self, name):
        self.name = name
        self.identifiers = "{}-{}".format(platform.node(), cname(name))
        self.manufacturer = "burundiocibu"
        self.sw_version = "v0.0.1"
        self.model = "virtual"

class EntityConfig:
    def __init__(self, name, device_class, device, topic):
        self.name = name
        self.device_class = device_class
        self.state_topic = f"{topic}/state"
        self.unique_id = "loranet-{}".format(cname(name))
        self.device = vars(device)

class BaseEntity:
    def __init__(self, name, device_class, entity_class, device, mqtt_client, root_topic="loranet"):
        self.name = name
        self.root_topic = root_topic
        self.mqtt_client = mqtt_client
        self.entity_class = entity_class
        self.device_class = device_class
        self.topic = "{}/{}/{}".format(self.root_topic, self.entity_class, cname(self.name))
        self.config = EntityConfig(self.name, self.device_class, device, self.topic)
        self.state = None

    def publish_discovery(self):
        ha_discovery_topic = "homeassistant/{}/{}/config".format(self.entity_class, cname(self.name))
        self.mqtt_client.publish(ha_discovery_topic, json.dumps(vars(self.config)))
        #print("{} {}".format(ha_discovery_topic, json.dumps(vars(self.config))))

    def publish_state(self, state=None):
        if state is not None:
            self.state = state
        self.mqtt_client.publish(self.config.state_topic, self.state)


class BatteryLevel(BaseEntity):
    def __init__(self, name, device, mqtt_client):
        super().__init__(name, "battery", "sensor", device, mqtt_client)
        self.config.unit_of_measurement = "%"
        self.publish_discovery()


class RSSI(BaseEntity):
    def __init__(self, name, device, mqtt_client):
        super().__init__(name, "signal_strength", "sensor", device, mqtt_client)
        self.config.unit_of_measurement = "dBm"
        self.publish_discovery()


class Sensor(BaseEntity):
    def __init__(self, name, device, mqtt_client, units=None):
        super().__init__(name, "None", "sensor", device, mqtt_client)
        if units is not None:
            self.config.unit_of_measurement = units
        self.publish_discovery()


class Gate(BaseEntity):
    def __init__(self, name, device, mqtt_client):
        super().__init__(name, "gate", "cover", device, mqtt_client)
        self.config.position_topic = f"{self.topic}/position"
        self.config.set_position_topic = f"{self.topic}/position/command"
        self.config.command_topic = f"{self.topic}/command"
        self.publish_discovery()
        self.state = 'closed' # or  open, opening, closed, closing, stopped
        self.position = 0

        self.mqtt_client.message_callback_add(self.config.command_topic, self.command)
        self.mqtt_client.subscribe(self.config.command_topic)
        self.mqtt_client.message_callback_add(self.config.set_position_topic, self.command)
        self.mqtt_client.subscribe(self.config.set_position_topic)

    def publish_state(self):
        self.mqtt_client.publish(self.config.state_topic, self.state)
        self.mqtt_client.publish(self.config.position_topic, self.position)

    # Callback for when we receive a message
    def command(self, client, obj, message):
        if message.topic == self.config.command_topic:
            if message.payload == b'OPEN':
                print("Opening")
                self.position = 100
            elif message.payload == b'CLOSE':
                print("Closing")
                self.position = 0
            elif message.payload == b'STOP':
                print("Nope")
        if message.topic == self.config.set_position_topic:
            self.position = int(message.payload)
            print(f"position = {int(message.payload)}")
        if self.position > 0:
            self.state = "open"
        else:
            self.state = "closed"
        self.publish_state()


def main():
    mqtt_username = os.environ.get("mqtt_username", "")
    mqtt_password = os.environ.get("mqtt_password", "")

    client = mqtt.Client()
    client.on_disconnect = on_disconnect

    if len(mqtt_username) and len(mqtt_password):
        client.username_pw_set(mqtt_username, mqtt_password)
    else:
        print("No username/password specified")

    client.connect("duckling.grootland")
    client.loop_start()

    driveway_gate_dev = DeviceConfig("Gate Manager")
    driveway_gate = Gate("Driveway Gate", driveway_gate_dev, client)
    driveway_gate_battery = BatteryLevel("Driveway Gate Battery", driveway_gate_dev, client)
    driveway_gate_rssi = RSSI("Driveway Gate RSSI", driveway_gate_dev, client)
    driveway_gate_tx_rtt = Sensor("Driveway Gate tx rtt", driveway_gate_dev, client, "ms")

    loranet_dev = DeviceConfig("loranet-bridge")
    loranet_uptime = Sensor("LoRaNet uptime", loranet_dev, client, "s")

    start_time = datetime.datetime.now()
    # messages:
    # S      return status message
    # P:pwr  set tx power
    # 
    while True:
        if not lora_tx("S"):
            continue

        packet = lora_rx(0.75)
        if packet is not None:
            display.fill(0)
            display.text(f"RSSI: {rfm9x.last_rssi} dB", 0, 10, 1)
            display.text(f"SNR: {rfm9x.last_snr} dB", 0, 20, 1)
            node = decode_msg(packet)
            display.show()
            driveway_gate_rssi.publish_state(rfm9x.last_rssi)
            driveway_gate_tx_rtt.publish_state(tx_stats['rtt'])

            pwr = int(node['txpwr'])
            if pwr != tx_power:
                tx(f"P:{tx_power}")

        driveway_gate.publish_state()
        driveway_gate_battery.publish_state(99)

        loranet_uptime.state = (datetime.datetime.now() - start_time).seconds
        loranet_uptime.publish_state()

        time.sleep(5)

if __name__ == "__main__":
    main()