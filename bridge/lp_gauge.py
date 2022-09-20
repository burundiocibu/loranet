#!/usr/bin/env python3
# -*- coding: utf-8 -*-
""" The central side othe the LPGauge LoRaNet node
"""

import logging
import os
import time

import entities

logger = logging.getLogger(__name__)


class LPGauge(entities.LoRaNode):
    def __init__(self, name, id, radio, mqtt_client):
        super().__init__(name, id, radio, mqtt_client)
        self.gate = entities.Gate(self.name, self.device_config, mqtt_client )

        self.feather_battery = entities.BatteryLevel(f"{self.name} LoRa Battery", self.device_config, mqtt_client)
        self.feather_battery_voltage = entities.Voltage(f"{self.name} LoRa Battery Voltage", self.device_config, mqtt_client)
        self.hall_voltage = entities.Voltage(f"{self.name} Hall Sensor Voltage", self.device_config, mqtt_client)
        self.lp_temperature = entities.Temperature(f"{self.name} Tank Temperature", self.device_config, mqtt_client)
        self.rssi = entities.RSSI(f"{self.name} RSSI", self.device_config, mqtt_client)
        self.rtt = entities.Sensor(f"{self.name} rtt", self.device_config, mqtt_client, "ms")
        self.uptime = entities.Sensor(f"{self.name} uptime", self.device_config, mqtt_client, "s")

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
            if 'hv' in msg:
                self.hall_voltage.publish_state(float(msg["hv"]))
            if 't1' in msg:
                self.lp_temperature.publish_state(float(msg["t1"]))
            if 'fvb' in msg:
                self.feather_battery.publish_state(int(100 * (float(msg["fvb"]) - 3.4) / (4.1 - 3.4)))
                self.feather_battery_voltage.publish_state(float(msg["fvb"]))
            if 'ut' in msg:
                self.uptime.publish_state(int(msg["ut"]))
            logger.debug(f"{self.name} state updated")
        except BaseException as e:
            logger.warning(f"Error processing packet:{packet}, {e}")
