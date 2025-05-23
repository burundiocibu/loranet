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

        self.actuator_position = entities.Distance(f"{self.name} Actuator Position", self.device_config, mqtt_client, "mm")

        self.set_actuator_position = entities.Number(f"{self.name} Set Actuator Position", self.device_config, mqtt_client, 0, 560, 0.25)
        self.mqtt_client.message_callback_add(self.set_actuator_position.config.command_topic, self.set_actuator_position_mqrx)
        self.mqtt_client.subscribe(self.set_actuator_position.config.command_topic)

        self.gate_closed_position = entities.Distance(f"{self.name} Actuator Closed Position", self.device_config, mqtt_client, "mm")
        self.actuator_loe = entities.Sensor(f"{self.name} Actuator LOE", self.device_config, mqtt_client, "cnt")
        self.arm_limit = entities.Binary(f"{self.name} Limit Switch", self.device_config, self.mqtt_client)
        self.remote_trigger = entities.Binary(f"{self.name} Remote Trigger", self.device_config, self.mqtt_client)

        self.gate_jog_open = entities.Button(f"{self.name} Jog Open", self.device_config, self.mqtt_client, icon="mdi:gate-open")
        self.mqtt_client.message_callback_add(self.gate_jog_open.config.command_topic, self.gate_jog_open_mqrx)
        self.mqtt_client.subscribe(self.gate_jog_open.config.command_topic)

        self.gate_jog_close = entities.Button(f"{self.name} Jog Close", self.device_config, self.mqtt_client, icon="mdi:gate")
        self.mqtt_client.message_callback_add(self.gate_jog_close.config.command_topic, self.gate_jog_close_mqrx)
        self.mqtt_client.subscribe(self.gate_jog_close.config.command_topic)

        self.gate_set_closed_position = entities.Button(f"{self.name} Set Closed Position", self.device_config, self.mqtt_client, icon="mdi:arrow-down")
        self.mqtt_client.message_callback_add(self.gate_set_closed_position.config.command_topic, self.gate_set_closed_position_mqrx)
        self.mqtt_client.subscribe(self.gate_set_closed_position.config.command_topic)

        self.scc_load_enable = entities.Switch(f"{self.name} SCC Load Enable", self.device_config, self.mqtt_client)
        self.mqtt_client.message_callback_add(self.scc_load_enable.config.command_topic, self.scc_load_enable_mqrx)
        self.mqtt_client.subscribe(self.scc_load_enable.config.command_topic)

        self.poe_enable = entities.Switch(f"{self.name} PoE Enable", self.device_config, self.mqtt_client)
        self.mqtt_client.message_callback_add(self.poe_enable.config.command_topic, self.poe_enable_mqrx)
        self.mqtt_client.subscribe(self.poe_enable.config.command_topic)

        self.rssi = entities.RSSI(f"{self.name} RSSI", self.device_config, mqtt_client)
        self.snr = entities.SNR(f"{self.name} SNR", self.device_config, mqtt_client)
        self.uptime = entities.Sensor(f"{self.name} uptime", self.device_config, mqtt_client, "s")
        self.wifi = entities.Sensor(f"{self.name} WiFI", self.device_config, mqtt_client)

        self.restart = entities.Button(f"{self.name} Restart", self.device_config, self.mqtt_client, icon="mdi:restart")
        self.mqtt_client.message_callback_add(self.restart.config.command_topic, self.restart_mqrx)
        self.mqtt_client.subscribe(self.restart.config.command_topic)

        self.scc_battery_voltage = entities.Voltage(f"{self.name} SCC Battery Voltage", self.device_config, mqtt_client)
        self.scc_battery_current = entities.Current(f"{self.name} SCC Battery Current", self.device_config, mqtt_client)
        self.scc_battery = entities.BatteryLevel(f"{self.name} SCC Battery", self.device_config, mqtt_client)
        self.scc_discharge_limit = entities.Voltage(f"{self.name} SCC Discharge Limit", self.device_config, mqtt_client)
        self.scc_load_power = entities.Power(f"{self.name} SCC Load Power", self.device_config, mqtt_client)
        self.scc_solar_power = entities.Power(f"{self.name} SCC Charge Power", self.device_config, mqtt_client)
        self.scc_charge_state = entities.Sensor(f"{self.name} SCC Charge State", self.device_config, mqtt_client)
        self.scc_temperature = entities.Temperature(f"{self.name} SCC Temperature", self.device_config, mqtt_client)

        self.mm_per_count = 0.25


    def update(self, msg):
        self.rssi.publish_state(self.radio.rfm9x.last_rssi)
        if 'ut' in msg:
            self.uptime.publish_state(int(msg["ut"]))

        if 'snr' in msg:
            self.snr.publish_state(int(msg["snr"]))

        if 'rssi' in msg:
            self.rssi.publish_state(int(msg["rssi"]))

        if 'bv' in msg:
            self.scc_battery_voltage.publish_state(float(msg["bv"]));
        
        if 'bc' in msg:
            self.scc_battery_current.publish_state(float(msg["bc"]));
        
        if 'bp' in msg:
            self.scc_battery.publish_state(float(msg["bp"]))

        if 'dl' in msg:
            self.scc_discharge_limit.publish_state(float(msg["dl"]))

        if 'wifi' in msg:
            self.wifi.publish_state(float(msg["wifi"]))

        if 'rr' in msg:
            self.remote_trigger.publish_state("ON")
        else:
            self.remote_trigger.publish_state("OFF")

        if 'al' in msg:
            if int(msg["al"]) == 1: self.arm_limit.publish_state("ON")
            else:                   self.arm_limit.publish_state("OFF")

        if 'cp' in msg:
            self.scc_solar_power.publish_state(round(float(msg["cp"]),2))

        if 'cs' in msg:
            self.scc_charge_state.publish_state(int(msg["cs"]))

        if 'lp' in msg:
            self.scc_load_power.publish_state(float(msg["lp"]))

        if 'ct' in msg:
            self.scc_temperature.publish_state(float(msg["ct"]))

        if 'lo' in msg:
            if int(msg["lo"]) == 0:  self.scc_load_enable.publish_state("OFF")
            else:                    self.scc_load_enable.publish_state("ON")

        if 'poe' in msg:
            if int(msg["poe"]) == 0:  self.poe_enable.publish_state("OFF")
            else:                     self.poe_enable.publish_state("ON")

        if 'ap' in msg:
            self.actuator_position.publish_state(round(self.mm_per_count*float(msg["ap"]),2))

        if 'acp' in msg:
            self.gate_closed_position.publish_state(round( self.mm_per_count*float(msg["acp"]),2))

        if 'aloe' in msg:
            self.actuator_loe.publish_state(int(msg["aloe"]))

        if 'gp' in msg and 'ms' in msg:
            self.gate.position = int(msg["gp"])
            motor_speed = int(msg['ms'])
            if motor_speed > 0:
                self.gate.state = "closing"
            elif motor_speed < 0:
                self.gate.state = "opening"
            elif self.gate.position == 0:
                self.gate.state = "closed"
            else:
                self.gate.state = "open"
            self.gate.publish_state()
        logger.debug("state updated")


    def gate_mqrx(self, mqtt_client, obj, message):
        if message.topic == self.gate.config.command_topic:
            if message.payload == b'OPEN':
                logger.info("Opening")
                self.radio.tx(self.id, "GP100")
            elif message.payload == b'CLOSE':
                logger.info("Closing")
                self.radio.tx(self.id, "GP0")
            elif message.payload == b'STOP':
                logger.info("Stopping")
                self.radio.tx(self.id, "GS")
        if message.topic == self.gate.config.set_position_topic:
            self.gate.position = int(message.payload)
            logger.info(f"position = {self.gate.position}")
            self.radio.tx(self.id, f"GP{self.gate.position}")

    def gate_jog_open_mqrx(self, mqtt_client, obj, message):
        logger.info(f"jog open")
        self.radio.tx(self.id, "G+")
        return

    def gate_jog_close_mqrx(self, mqtt_client, obj, message):
        logger.info(f"jog close")
        self.radio.tx(self.id, "G-")
        return
    
    def set_actuator_position_mqrx(self, mqtt_client, obj, message):
        pos = int(float(message.payload)/self.mm_per_count)
        logger.info(f"set_actuator position {message.payload} {pos}")
        self.radio.tx(self.id, f"AP{pos}")
        return

    def gate_set_closed_position_mqrx(self, mqtt_client, obj, message):
        logger.info(f"set closed position")
        self.radio.tx(self.id, "GSCP")
        return

    def scc_load_enable_mqrx(self, mqtt_client, obj, message):
        if message.payload == b"ON":
            logger.info("scc load on")
            self.radio.tx(self.id, "R1")
        elif message.payload == b"OFF":
            logger.info("scc load off")
            self.radio.tx(self.id, "R0")
        else:
            logger.warning(f"Received unexpected mqtt message: {message.payload}")

    def poe_enable_mqrx(self, mqtt_client, obj, message):
        if message.payload == b"ON":
            logger.info("PoE on")
            self.radio.tx(self.id, "POE1")
        elif message.payload == b"OFF":
            logger.info("PoE off")
            self.radio.tx(self.id, "POE0")
        else:
            logger.warning(f"Received unexpected mqtt message: {message.payload}")

    def restart_mqrx(self, mqtt_client, obj, message):
        logger.info(f"restart")
        self.radio.tx(self.id, "restart")
        return
