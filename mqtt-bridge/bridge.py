#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from audioop import add
from copyreg import add_extension
import datetime
from http.client import GATEWAY_TIMEOUT
import json
import logging
import os
import platform
from socket import MsgFlag

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
display.text('LoRaNet', 0, 0, 1)
display.show()

logger = logging.getLogger(__name__)

class LoRaBase():
    def __init__(self):
        # Configure LoRa Radio, https://circuitpython.readthedocs.io/projects/rfm9x/en/latest/
        # https://github.com/adafruit/Adafruit_CircuitPython_RFM9x
        # http://www.airspayce.com/mikem/arduino/RadioHead/classRH__RF95.html

        CS = DigitalInOut(board.CE1)
        RESET = DigitalInOut(board.D25)
        self.spi = busio.SPI(board.SCK, MOSI=board.MOSI, MISO=board.MISO)
        self.rfm9x = adafruit_rfm9x.RFM9x(self.spi, CS, RESET, 915.0) # 915 Mhz
        self.tx_power = 5
        self.rfm9x.tx_power = self.tx_power
        self.rfm9x.node = 1
        self.rfm9x.flags = 0 # default is 0
        self.tx_stats = {'count':0, 'err':0 }
        self.rx_stats = {'count':0, 'err':0 }
        self.msg = None # most recent received message

    def print_stats(self, msg:str) -> None:
        logger.debug(f"{msg}  tx:{self.tx_stats}  rx:{self.rx_stats}  msg:{self.msg}")

    def tx(self, dest, msg:str) -> bool:
        t0 = time.perf_counter()
        self.rfm9x.destination = dest #  default is 255 (broadcast) 
        self.tx_stats['count'] += 1
        if not self.rfm9x.send_with_ack(bytes(msg, "utf-8")):
            self.tx_stats['err'] += 1
            self.print_stats("noack")
            return False
        self.tx_stats['rtt'] = round(1000 * (time.perf_counter() - t0), 1)
        self.tx_stats['rssi'] = self.rfm9x.last_rssi
        self.tx_stats['snr'] = self.rfm9x.last_snr
        self.tx_stats['txpwr'] = self.rfm9x.tx_power
        if True or self.tx_stats['count'] % 10 == 0:
            self.print_stats("ok")
        return True

    def rx(self, sender, timeout:float = 0.5):
        t0 = time.perf_counter()
        self.rfm9x.receive_timeout = timeout # seconds
        self.rfm9x.destination = sender 
        packet = self.rfm9x.receive(with_ack=True)
        self.rx_stats['rtt'] = round(1000 * (time.perf_counter() - t0),)
        self.msg = packet
        if packet is None:
            self.rx_stats['err'] += 1
            self.print_stats('timeout')
            return None
        self.rx_stats['count'] += 1
        return packet
        
    # note this has to be seperate since it can't ack and broadcast
    def broadcast(self, msg:str) -> None:
        self.rfm9x.destination = 255 #  default is 255 (broadcast) 
        self.rfm9x.send(bytes(msg, "utf-8"))

    # note this has to be seperate since it can't ack and broadcast
    def listen(self, timeout:float = 1):
        self.rfm9x.destination = 255 #  default is 255 (broadcast) 
        self.rfm9x.receive_timeout = timeout # seconds
        packet = self.rfm9x.recive(with_header=True)
        return int(packet[1]), packet[4:]

    def decode_msg(self, msg:str) -> dict:
        d = {}
        for s in msg.decode().split(","):
            if len(s):
                n,v = s.split(':')
                d[n] = v
        return d


def on_disconnect():
    logger.info(" mqtt disconnecting...")


def on_connect(mqttc, obj, flags, rc):
    logger.info("mqtt connect rc: " + str(rc))


def cname(string:str) -> str:
    return string.replace(" ","-").lower()


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
        if device_class is not None:
            self.device_class = device_class
        self.state_topic = f"{topic}/state"
        self.unique_id = "loranet-{}".format(cname(name))
        self.device = vars(device)

class BaseEntity:
    def __init__(self, name, device_class, entity_class, 
    device, mqtt_client, root_topic="loranet"):
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
        msg = json.dumps(vars(self.config))
        logger.debug(f"discovery topic: {ha_discovery_topic}")
        logger.debug(f"discovery msg: {msg}")
        self.mqtt_client.publish(ha_discovery_topic, msg)

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


class Timestamp(BaseEntity):
    def __init__(self, name, device, mqtt_client, units=None):
        super().__init__(name, "timestamp", "sensor", device, mqtt_client)
        self.publish_discovery()


class Sensor(BaseEntity):
    def __init__(self, name, device, mqtt_client, units=None):
        super().__init__(name, None, "sensor", device, mqtt_client)
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

    def publish_state(self):
        self.mqtt_client.publish(self.config.state_topic, self.state)
        self.mqtt_client.publish(self.config.position_topic, self.position)


class LoRaNode():
    def __init__(self, name:str, id:int, radio, client):
        self.id = id
        self.name = name
        self.radio = radio
        self.client = client
        self.device_config = DeviceConfig(f"{self.name} Manager")

    def update_state(self):
        """update and publish the state of all entities"""
        pass


class LoRaNetBridge(LoRaNode):
    def __init__(self, radio, client):
        super().__init__("LoRaNet Bridge", 0, radio, client)
        self.device_config = DeviceConfig(f"{self.name}")
        self.uptime = Sensor(f"{self.name} uptime", self.device_config, client, "s")
        self.start_time = datetime.datetime.now()

    def update_state(self):
        self.uptime.publish_state((datetime.datetime.now() - self.start_time).seconds)


class DrivewayGate(LoRaNode):
    def __init__(self, name, radio, client):
        super().__init__(name, 2, radio, client)
        self.gate = Gate(self.name, self.device_config, client)
        self.battery = BatteryLevel(f"{self.name} Battery", self.device_config, client)
        self.rssi = RSSI(f"{self.name} RSSI", self.device_config, client)
        self.tx_rtt = Sensor(f"{self.name} tx rtt", self.device_config, client, "ms")
        self.uptime = Sensor(f"{self.name} uptime", self.device_config, client, "s")
        # messages this device handle
        self.client.message_callback_add(self.gate.config.command_topic, self.command)
        self.client.subscribe(self.gate.config.command_topic)
        self.client.message_callback_add(self.gate.config.set_position_topic, self.command)
        self.client.subscribe(self.gate.config.set_position_topic)

    def update_state(self):
        if not self.radio.tx(self.id, "S"):
            return
        self.receive_status()

    def receive_status(self):
        packet = self.radio.rx(sender=self.id, timeout=0.75)
        if packet is not None:
            msg = self.radio.decode_msg(packet)
            logger.info(f"msg:{msg}")
            self.rssi.publish_state(self.radio.rfm9x.last_rssi)
            self.tx_rtt.publish_state(self.radio.tx_stats['rtt'])
            self.battery.publish_state(int(100 * float(msg["fvb"]) / 4.2))
            self.uptime.publish_state(int(msg["ut"]))
            self.gate.position = int(msg["gpos"])
            if self.gate.position > 0:
                self.gate.state = "open"
            else:
                self.gate.state = "closed"
            self.gate.publish_state()

    # Callback for when we receive a message
    def command(self, client, obj, message):
        if message.topic == self.gate.config.command_topic:
            if message.payload == b'OPEN':
                logger.debug("Opening")
                self.radio.tx(2, "O")
            elif message.payload == b'CLOSE':
                logger.debug("Closing")
                self.radio.tx(2, "C")
            elif message.payload == b'STOP':
                logger.debug("Nope")
        if message.topic == self.gate.config.set_position_topic:
            self.gate.position = int(message.payload)
            logger.debug(f"position = {self.gate.position}")
            self.radio.tx(2, f"K{self.gate.position}")
        if self.gate.position > 0:
            self.gate.state = "open"
        else:
            self.gate.state = "closed"
        self.receive_status()


def run(client, radio):
    client.loop_start()

    loranet_bridge = LoRaNetBridge(radio, client)
    driveway_gate = DrivewayGate("Driveway Gate", radio, client)

    # see readme for a dictionary of messages
    while True:
        loranet_bridge.update_state()
        driveway_gate.update_state()
        time.sleep(15)


def main():
    # Eventually may put argparse/config file processing here.
    logging.basicConfig(format="%(asctime)s %(threadName)s: %(message)s", 
        level=logging.INFO)

    radio = LoRaBase()

    mqtt_username = os.environ.get("mqtt_username", "")
    mqtt_password = os.environ.get("mqtt_password", "")

    client = mqtt.Client()
    client.on_disconnect = on_disconnect
    client.on_connect = on_connect

    if len(mqtt_username) and len(mqtt_password):
        client.username_pw_set(mqtt_username, mqtt_password)
    else:
        logger.error("No username/password specified")
        return

    client.connect("duckling.grootland")

    run(client, radio)


if __name__ == "__main__":
    main()