#!/usr/bin/env python3
# -*- coding: utf-8 -*-
""" The central side of the the DrivewayGate LoRaNet node
"""

import logging
import os
import time

import entities

logger = logging.getLogger(__name__)


class DrivewayGate(entities.LoRaNode):
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

        self.gate_enable = entities.Switch(f"{self.name} Enable", self.device_config, self.mqtt_client)
        self.mqtt_client.message_callback_add(self.gate_enable.config.command_topic, self.gate_enable_mqrx)
        self.mqtt_client.subscribe(self.gate_enable.config.command_topic)

        self.rover_load_enable = entities.Switch(f"{self.name} Rover Load Enable", self.device_config, self.mqtt_client)
        self.mqtt_client.message_callback_add(self.rover_load_enable.config.command_topic, self.rover_mqrx)
        self.mqtt_client.subscribe(self.rover_load_enable.config.command_topic)

        self.feather_battery = entities.BatteryLevel(f"{self.name} LoRa Battery", self.device_config, mqtt_client)
        self.feather_battery_voltage = entities.Voltage(f"{self.name} LoRa Battery Voltage", self.device_config, mqtt_client)
        self.gate_battery = entities.BatteryLevel(f"{self.name} Battery", self.device_config, mqtt_client)
        self.gate_battery_voltage = entities.Voltage(f"{self.name} Battery Voltage", self.device_config, mqtt_client)
        self.rssi = entities.RSSI(f"{self.name} RSSI", self.device_config, mqtt_client)
        self.rtt = entities.Sensor(f"{self.name} rtt", self.device_config, mqtt_client, "ms")
        self.uptime = entities.Sensor(f"{self.name} uptime", self.device_config, mqtt_client, "s")

        self.rover_battery_voltage = entities.Voltage(f"{self.name} Rover Battery Voltage", self.device_config, mqtt_client)
        self.rover_battery_current = entities.Current(f"{self.name} Rover Battery Current", self.device_config, mqtt_client)
        self.rover_battery = entities.BatteryLevel(f"{self.name} Rover Battery", self.device_config, mqtt_client)

        self.rover_load_voltage = entities.Voltage(f"{self.name} Rover Load Voltage", self.device_config, mqtt_client)
        self.rover_load_current = entities.Current(f"{self.name} Rover Load Current", self.device_config, mqtt_client)
        self.rover_load_power = entities.Power(f"{self.name} Rover Load Power", self.device_config, mqtt_client)

        self.rover_solar_power = entities.Power(f"{self.name} Rover Solar Power", self.device_config, mqtt_client)
        self.rover_solar_voltage = entities.Voltage(f"{self.name} Rover Solar Voltage", self.device_config, mqtt_client)
        self.rover_solar_current = entities.Current(f"{self.name} Rover Solar Current", self.device_config, mqtt_client)
        self.rover_charge_state = entities.Sensor(f"{self.name} Rover Charge State", self.device_config, mqtt_client)
        self.rover_temperature = entities.Temperature(f"{self.name} Rover Temperature", self.device_config, mqtt_client)

    def update_state(self):
        logger.info("Requesting state")
        if not self.radio.tx(self.id, "?"):
            return

    def receive_status(self, packet):
        try:
            msg = self.radio.decode_msg(packet)
            self.rssi.publish_state(self.radio.rfm9x.last_rssi)
            if 'rtt' in msg:
                self.rtt.publish_state(int(msg['rtt']))
            if 'fvb' in msg:
                self.feather_battery.publish_state(int(100 * (float(msg["fvb"]) - 3.4) / (4.19 - 3.4)))
                self.feather_battery_voltage.publish_state(float(msg["fvb"]))
            if 'gvb' in msg:
                self.gate_battery.publish_state(int(100 * (float(msg["gvb"]) - 10.5) / (14.14 - 10.5)))
                self.gate_battery_voltage.publish_state(float(msg["gvb"]))
            if 'ut' in msg:
                self.uptime.publish_state(int(msg["ut"]))

            if 'bv' in msg:
                self.rover_battery_voltage.publish_state(float(msg["bv"]));
            if 'bc' in msg:
                self.rover_battery_current.publish_state(float(msg["bc"]));
            if 'bp' in msg:
                self.rover_battery.publish_state(float(msg["bp"]))

            if 'lv' in msg and 'lc' in msg:
                self.rover_load_voltage.publish_state(round(float(msg["lv"]),2))
                self.rover_load_current.publish_state(round(float(msg["lc"]),2)) # for some reason this reads high
                self.rover_load_power.publish_state(round(float(msg["lv"]) * float(msg["lc"]),2))

            if 'sv' in msg and 'sc' in msg:
                self.rover_solar_voltage.publish_state(round(float(msg["sv"]),2))
                self.rover_solar_current.publish_state(round(float(msg["sc"]),2))
                self.rover_solar_power.publish_state(round(float(msg["sv"]) * float(msg["sc"]),2))

            if 'cs' in msg:
                self.rover_charge_state.publish_state(int(msg["cs"]))

            if 'ct' in msg:
                self.rover_temperature.publish_state(float(msg["ct"]))

            if 'poe' in msg:
                if int(msg["poe"]) == 0:  self.poe_enable.state = "OFF"
                else:                     self.poe_enable.state = "ON"
                self.poe_enable.publish_state()

            if 'ge' in msg:
                if int(msg["ge"]) == 0:  self.gate_enable.state = "OFF"
                else:                    self.gate_enable.state = "ON"
                self.gate_enable.publish_state()

            if 'lo' in msg:
                if int(msg["lo"]) == 0:  self.rover_load_enable.state = "OFF"
                else:                    self.rover_load_enable.state = "ON"
                self.rover_load_enable.publish_state()

            if 'gp' in msg:
                self.gate.position = int(msg["gp"])
                if self.gate.position > 0:
                    self.gate.state = "open"
                else:
                    self.gate.state = "closed"
                self.gate.publish_state()
            logger.debug(f"{self.name} state updated")
        except BaseException as e:
            logger.warning(f"Error processing packet:{packet}, {e}")

    def gate_mqrx(self, mqtt_client, obj, message):
        if message.topic == self.gate.config.command_topic:
            if message.payload == b'OPEN':
                logger.info("Opening")
                self.radio.tx(self.id, "GO")
            elif message.payload == b'CLOSE':
                logger.info("Closing")
                self.radio.tx(self.id, "GC")
            elif message.payload == b'STOP':
                logger.info("Nope")
        if message.topic == self.gate.config.set_position_topic:
            self.gate.position = int(message.payload)
            logger.info(f"position = {self.gate.position}")
            self.radio.tx(self.id, f"K{self.gate.position}")

    def poe_mqrx(self, mqtt_client, obj, message):
        if message.payload == b"ON":
            logger.info("poe on")
            self.radio.tx(self.id, "E1")
        elif message.payload == b"OFF":
            logger.info("poe off")
            self.radio.tx(self.id, "E0")
        else:
            logger.warning(f"Received unexpected mqtt message: {message.payload}")
            return

    def gate_enable_mqrx(self, mqtt_client, obj, message):
        if message.payload == b"ON":
            logger.info("gate enable on")
            self.radio.tx(self.id, "GE1")
        elif message.payload == b"OFF":
            logger.info("gate enable off")
            self.radio.tx(self.id, "GE0")
        else:
            logger.warning(f"Received unexpected mqtt message: {message.payload}")
            return

    def rover_mqrx(self, mqtt_client, obj, message):
        if message.payload == b"ON":
            logger.info("rover load on")
            self.radio.tx(self.id, "R1")
        elif message.payload == b"OFF":
            logger.info("rover load off")
            self.radio.tx(self.id, "R0")
        else:
            logger.warning(f"Received unexpected mqtt message: {message.payload}")
