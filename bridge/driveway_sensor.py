#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
import os
import time

import entities

logger = logging.getLogger(__name__)


class DrivewaySensor(entities.LoRaNode):
    def __init__(self, name, id, radio, mqtt_client):
        super().__init__(name, id, radio, mqtt_client)

        self.battery = entities.BatteryLevel(f"{self.name} Battery", self.device_config, mqtt_client)
        self.battery_voltage = entities.Voltage(f"{self.name} Battery Voltage", self.device_config, mqtt_client)
        self.rssi = entities.RSSI(f"{self.name} LoRa RSSI", self.device_config, mqtt_client)
        self.uptime = entities.Sensor(f"{self.name} uptime", self.device_config, mqtt_client, "s")
        self.presence = entities.Presense(f"{self.name} Presense", self.device_config, mqtt_client)
        
        self.presense_timeout = 5 # seconds since detection till presense removed
        self.last_presense = time.monotonic();

    def update(self, msg):
        self.radio.tx(self.id, "ack")
        self.presence.publish_state("ON")
        self.rssi.publish_state(self.radio.rfm9x.last_rssi)
        if 'vb' in msg:
            self.battery.publish_state(int(100 * (float(msg["vb"]) - 3.4) / (4.28 - 3.4)))
            self.battery_voltage.publish_state(float(msg["vb"]))
        if 'ut' in msg:
            self.uptime.publish_state(int(msg["ut"]))
        logger.debug("state updated")
    
    def checkin(self):
        if time.monotonic() - self.last_state_update > self.presense_timeout and self.presence.state == "ON":
            logger.debug(f"state:{self.presence.state}, clearing")
            self.presence.publish_state("OFF")
