#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# These started as templates for entites to set up auto discovery in home assistant
# Now they are going to become a prometheus endpoint

import json
import logging
import platform
import time
from prometheus_client import start_http_server, Gauge


logger = logging.getLogger(__name__)

basename = "loranet"
port=9433
start_http_server(port)

gauge = Gauge(f"{basename}", "Entities from LoRaNet", ['entity', 'units'])

def cname(string:str) -> str:
    return string.replace(" ","-").lower()


class DeviceConfig:
    def __init__(self, name):
        self.name = name
        self.identifiers = "{}-{}".format(platform.node(), cname(name))
        self.manufacturer = "burundiocibu"
        self.sw_version = "v0.0.1"
        self.model = "virtual"


class LoRaNode():
    def __init__(self, name:str, id:int, radio, mqtt_client):
        self.id = id
        self.name = name
        self.radio = radio
        self.mqtt_client = mqtt_client
        self.device_config = DeviceConfig(f"{self.name} Manager")

        self.update_rate = 60
        self.last_state_update = time.monotonic() - self.update_rate

    def update(msg):
        pass

    def update_state(self, sender, packet):
        if sender != self.id:
            return
        try:
            if packet[0:5] == "text:":
                logger.info(packet)
            else:
                self.last_state_update = time.monotonic()
                msg = self.radio.decode_msg(packet)
                self.update(msg)
        except BaseException as e:
            logger.warning(f"Error processing packet:{packet}, {e}")
    
    def request_state(self):
        if time.monotonic() - self.last_state_update > self.update_rate and self.radio.free_to_send():
            logger.debug(f"{self.name} requesting state")
            self.radio.tx(self.id, "SS")



# See https://www.home-assistant.io/docs/configuration/customizing-devices/#device-class
# for the device classes
class EntityConfig:
    def __init__(self, name, device_class, device, topic, icon):
        self.name = name
        if device_class is not None:
            self.device_class = device_class
        self.state_topic = f"{topic}/state"
        self.unique_id = "{}-{}".format(basename, cname(name))
        self.device = vars(device)
        if icon is not None:
            self.icon = icon


class BaseEntity:
    def __init__(self, name, device_class, entity_class, device, mqtt_client, root_topic=basename, icon=None):
        self.name = name
        self.cname = cname(self.name)
        self.root_topic = root_topic
        self.mqtt_client = mqtt_client
        self.entity_class = entity_class
        self.device_class = device_class
        self.topic = f"{self.root_topic}/{self.entity_class}/{self.cname}"
        self.config = EntityConfig(self.name, self.device_class, device, self.topic, icon)
        self.state = None

    def publish_discovery(self):
        ha_discovery_topic = "homeassistant/{}/{}/config".format(self.entity_class, cname(self.name))
        msg = json.dumps(vars(self.config))
        logger.debug(f"discovery topic: {ha_discovery_topic}")
        logger.debug(f"discovery msg: {msg}")
        self.mqtt_client.publish(ha_discovery_topic, msg, retain=True)

    def publish_state(self, state=None):
        if state is not None:
            self.state = state
        self.mqtt_client.publish(self.config.state_topic, self.state)
        
        units="-"
        if hasattr(self.config, 'unit_of_measurement'):
            units=self.config.unit_of_measurement
        state = self.state
        if state=="on" or state=="ON":
            state = 1
        elif state=="off" or state=="OFF":
            state = 0
        gauge.labels(entity=self.cname, units=units).set(state)


#==================
# Read only entites
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

class SNR(BaseEntity):
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
        self.config.unit_of_measurement = "°C"
        self.publish_discovery()

class Distance(BaseEntity):
    def __init__(self, name, device, mqtt_client, units=None):
        super().__init__(name, "distance", "sensor", device, mqtt_client)
        if units is not None:
            self.config.unit_of_measurement = units
        self.publish_discovery()

class Presense(BaseEntity):
    def __init__(self, name, device, mqtt_client):
        super().__init__(name, "presense", "binary_sensor", device, mqtt_client)
        #self.config.state_class = "measurement"
        del self.config.device_class
        self.publish_discovery()

class Binary(BaseEntity):
    def __init__(self, name, device, mqtt_client):
        super().__init__(name, "None", "binary_sensor", device, mqtt_client)
        del self.config.device_class
        self.publish_discovery()

class Sensor(BaseEntity):
    def __init__(self, name, device, mqtt_client, units=None):
        super().__init__(name, None, "sensor", device, mqtt_client)
        if units is not None:
            self.config.unit_of_measurement = units
        self.publish_discovery()


#=====================================
# Below are the interactive entities
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
        gauge.labels(entity=self.cname, units="").set(self.position)


class Switch(BaseEntity):
    def __init__(self, name, device, mqtt_client):
        super().__init__(name, "switch", "switch", device, mqtt_client)
        self.config.command_topic = f"{self.topic}/command"
        self.publish_discovery()
        self.state = "off"


class Button(BaseEntity):
    def __init__(self, name, device, mqtt_client, icon):
        super().__init__(name, "button", "button", device, mqtt_client, icon)
        self.config.command_topic = f"{self.topic}/command"
        self.config.acty_t = f"{self.topic}/status"
        del self.config.device_class
        del self.config.state_topic
        self.publish_discovery()


class Number(BaseEntity):
    def __init__(self, name, device, mqtt_client, min=0, max=1000, step=1):
        super().__init__(name, "number", "number", device, mqtt_client)
        self.config.command_topic = f"{self.topic}/command"
        self.config.acty_t = f"{self.topic}/status"
        self.config.min = min
        self.config.max = max
        self.config.step = step
        del self.config.device_class
        self.publish_discovery()
