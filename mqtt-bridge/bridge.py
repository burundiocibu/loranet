#!/usr/bin/env python3
# -*- coding: utf-8 -*-
""" A bridge between LoRaNet devices and MQTT. The local radio is an Adafruit LoRa bonnet for the raspberry pi
"""

import datetime
import logging
import os
import paho.mqtt.client as mqtt
import time

from lorabase import LoRaBase
from entities import *

logger = logging.getLogger(__name__)


def on_disconnect():
    logger.info(" mqtt disconnecting...")


def on_connect(mqttc, obj, flags, rc):
    logger.info("mqtt connect rc: " + str(rc))



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
        if not self.radio.tx(self.id, "R"):
            return
        self.receive_rover_status()
        

    def receive_status(self):
        packet = self.radio.rx(sender=self.id, timeout=0.75)
        if packet is not None:
            msg = self.radio.decode_msg(packet)
            logger.info(f"msg:{msg}")
            self.rssi.publish_state(self.radio.rfm9x.last_rssi)
            self.tx_rtt.publish_state(self.radio.tx_stats['rtt'])
            self.battery.publish_state(int(100 * (float(msg["fvb"]) - 3.4) / 0.8))
            self.uptime.publish_state(int(msg["ut"]))
            self.gate.position = int(msg["gpos"])
            if self.gate.position > 0:
                self.gate.state = "open"
            else:
                self.gate.state = "closed"
            self.gate.publish_state()

    def receive_rover_status(self):
        packet = self.radio.rx(sender=self.id, timeout=0.75)
        if packet is not None:
            msg = self.radio.decode_msg(packet)
            logger.info(f"msg:{msg}")

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
    logging.basicConfig(format="%(asctime)s %(threadName)s: %(message)s", level=logging.INFO)

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

    client.connect("duckling.groot")

    run(client, radio)


if __name__ == "__main__":
    main()