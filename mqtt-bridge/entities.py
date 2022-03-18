#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import logging
import platform

logger = logging.getLogger(__name__)

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

class Voltage(BaseEntity):
    def __init__(self, name, device, mqtt_client):
        super().__init__(name, "voltage", "sensor", device, mqtt_client)
        self.config.unit_of_measurement = "V"
        self.publish_discovery()

class Current(BaseEntity):
    def __init__(self, name, device, mqtt_client):
        super().__init__(name, "current", "sensor", device, mqtt_client)
        self.config.unit_of_measurement = "A"
        self.publish_discovery()

class Power(BaseEntity):
    def __init__(self, name, device, mqtt_client):
        super().__init__(name, "power", "sensor", device, mqtt_client)
        self.config.unit_of_measurement = "W"
        self.publish_discovery()

class Temperature(BaseEntity):
    def __init__(self, name, device, mqtt_client):
        super().__init__(name, "temperature", "sensor", device, mqtt_client)
        self.config.unit_of_measurement = "C"
        self.publish_discovery()


class Sensor(BaseEntity):
    def __init__(self, name, device, mqtt_client, units=None):
        super().__init__(name, None, "sensor", device, mqtt_client)
        if units is not None:
            self.config.unit_of_measurement = units
        self.publish_discovery()
