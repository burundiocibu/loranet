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
import entities

logger = logging.getLogger(__name__)


def on_disconnect():
    logger.info(" mqtt disconnecting...")


def on_connect(mqttc, obj, flags, rc):
    logger.info("mqtt connect rc: " + str(rc))


class LoRaNode():
    def __init__(self, name:str, id:int, radio, mqtt_client):
        self.id = id
        self.name = name
        self.radio = radio
        self.mqtt_client = mqtt_client
        self.device_config = entities.DeviceConfig(f"{self.name} Manager")

    def update_state(self):
        """update and publish the state of all entities"""
        pass


class LoRaNetBridge(LoRaNode):
    def __init__(self, id, radio, mqtt_client):
        super().__init__("LoRaNet Bridge", id, radio, mqtt_client)
        self.device_config = entities.DeviceConfig(f"{self.name}")
        self.uptime = entities.Sensor(f"{self.name} uptime", self.device_config, mqtt_client, "s")
        self.start_time = datetime.datetime.now()

    def update_state(self):
        self.uptime.publish_state((datetime.datetime.now() - self.start_time).seconds)


class DrivewayGate(LoRaNode):
    def __init__(self, name, id, radio, mqtt_client):
        super().__init__(name, id, radio, mqtt_client)
        self.gate = entities.Gate(self.name, self.device_config, mqtt_client )
        self.mqtt_client.message_callback_add(self.gate.config.command_topic, self.gate_mqrx)
        self.mqtt_client.subscribe(self.gate.config.command_topic)
        self.mqtt_client.message_callback_add(self.gate.config.set_position_topic, self.gate_mqrx)
        self.mqtt_client.subscribe(self.gate.config.set_position_topic)

        self.poe_enable = entities.Switch(f"{self.name} PoE Enable", self.device_config, self.mqtt_client)
        self.mqtt_client.message_callback_add(self.poe_enable.config.command_topic, self.poe_mqrx)
        self.mqtt_client.subscribe(self.poe_enable.config.command_topic)

        self.rover_load_enable = entities.Switch(f"{self.name} Rover Load Enable", self.device_config, self.mqtt_client)
        self.mqtt_client.message_callback_add(self.rover_load_enable.config.command_topic, self.rover_mqrx)
        self.mqtt_client.subscribe(self.rover_load_enable.config.command_topic)

        self.feather_battery = entities.BatteryLevel(f"{self.name} LoRa Battery", self.device_config, mqtt_client)
        self.gate_battery = entities.BatteryLevel(f"{self.name} Battery", self.device_config, mqtt_client)
        self.rssi = entities.RSSI(f"{self.name} RSSI", self.device_config, mqtt_client)
        self.tx_rtt = entities.Sensor(f"{self.name} tx rtt", self.device_config, mqtt_client, "ms")
        self.uptime = entities.Sensor(f"{self.name} uptime", self.device_config, mqtt_client, "s")
        self.rover_battery_voltage = entities.Voltage(f"{self.name} Rover Battery Voltage", self.device_config, mqtt_client)
        self.rover_battery = entities.BatteryLevel(f"{self.name} Rover Battery", self.device_config, mqtt_client)
        self.rover_load_power = entities.Power(f"{self.name} Rover Load Power", self.device_config, mqtt_client)
        self.rover_solar_power = entities.Power(f"{self.name} Rover Solar Power", self.device_config, mqtt_client)
        self.rover_charge_state = entities.Sensor(f"{self.name} Rover Charge State", self.device_config, mqtt_client)

    def update_state(self):
        logger.info("Requesting state")
        if not self.radio.tx(self.id, "S"):
            return
        self.receive_status()

    def receive_status(self):
        packet = self.radio.rx(sender=self.id, timeout=0.75)
        if packet is not None:
            msg = self.radio.decode_msg(packet)
            self.rssi.publish_state(self.radio.rfm9x.last_rssi)
            self.tx_rtt.publish_state(self.radio.tx_stats['rtt'])
            self.feather_battery.publish_state(int(100 * (float(msg["fvb"]) - 3.4) / (4.3 - 3.4)))
            self.gate_battery.publish_state(int(100 * (float(msg["gvb"]) - 10.5) / (12.8 - 10.5)))
            self.uptime.publish_state(int(msg["ut"]))
            self.rover_battery_voltage.publish_state(float(msg["bv"]));
            self.rover_battery.publish_state(float(msg["bp"]))
            self.rover_load_power.publish_state(round(float(msg["lv"]) * float(msg["lc"]),2))
            self.rover_solar_power.publish_state(round(float(msg["sv"]) * float(msg["sc"]),2))
            self.rover_charge_state.publish_state(int(msg["cs"]))
            if int(msg["poe"]) == 0:  self.poe_enable.state = "OFF"
            else:                     self.poe_enable.state = "ON"
            self.poe_enable.publish_state()
            if int(msg["lo"]) == 0:  self.rover_load_enable.state = "OFF"
            else:                    self.rover_load_enable.state = "ON"
            self.poe_enable.publish_state()

            self.gate.position = int(msg["gp"])
            if self.gate.position > 0:
                self.gate.state = "open"
            else:
                self.gate.state = "closed"
            self.gate.publish_state()

    def gate_mqrx(self, mqtt_client, obj, message):
        if message.topic == self.gate.config.command_topic:
            if message.payload == b'OPEN':
                logger.debug("Opening")
                self.radio.tx(self.id, "GO")
            elif message.payload == b'CLOSE':
                logger.debug("Closing")
                self.radio.tx(self.id, "GC")
            elif message.payload == b'STOP':
                logger.debug("Nope")
        if message.topic == self.gate.config.set_position_topic:
            self.gate.position = int(message.payload)
            logger.debug(f"position = {self.gate.position}")
            self.radio.tx(self.id, f"K{self.gate.position}")
        if self.gate.position > 0:
            self.gate.state = "open"
        else:
            self.gate.state = "closed"
        self.update_state()

    def poe_mqrx(self, mqtt_client, obj, message):
        if message.payload == b"ON":
            logger.debug("poe on")
            self.radio.tx(self.id, "E1")
        elif message.payload == b"OFF":
            logger.debug("poe off")
            self.radio.tx(self.id, "E0")
        else:
            logger.warning(f"Received unexpected mqtt message: {message.payload}")
            return
        self.update_state()

    def rover_mqrx(self, mqtt_client, obj, message):
        if message.payload == b"ON":
            logger.debug("rover load on")
            self.radio.tx(self.id, "R1")
        elif message.payload == b"OFF":
            logger.debug("rover load off")
            self.radio.tx(self.id, "R0")
        else:
            logger.warning(f"Received unexpected mqtt message: {message.payload}")
            return
        self.update_state()


def run(mqtt_client, radio):
    mqtt_client.loop_start()

    loranet_bridge = LoRaNetBridge(0, radio, mqtt_client)
    driveway_gate = DrivewayGate("Driveway Gate", 2, radio, mqtt_client)

    # see readme for a dictionary of messages
    while True:
        loranet_bridge.update_state()
        driveway_gate.update_state()
        time.sleep(15)


def main():
    # Eventually may put argparse/config file processing here.
    logging.basicConfig(format="%(asctime)s %(threadName)s: %(message)s", level=logging.DEBUG)

    radio = LoRaBase()

    mqtt_username = os.environ.get("mqtt_username", "")
    mqtt_password = os.environ.get("mqtt_password", "")

    mqtt_client = mqtt.Client()
    mqtt_client.on_disconnect = on_disconnect
    mqtt_client.on_connect = on_connect

    if len(mqtt_username) and len(mqtt_password):
        mqtt_client.username_pw_set(mqtt_username, mqtt_password)
    else:
        logger.error("No username/password specified")
        return

    mqtt_client.connect("duckling.groot")

    run(mqtt_client, radio)


if __name__ == "__main__":
    main()