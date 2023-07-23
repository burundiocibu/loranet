#!/usr/bin/env python3
# -*- coding: utf-8 -*-
""" A bridge between LoRaNet devices and MQTT. The local radio is an Adafruit LoRa bonnet for the raspberry pi
"""

import argparse
import datetime
import logging
import os
import paho.mqtt.client as mqtt
import time
import sys

from lorabase import LoRaBase
import entities
from driveway_gate import DrivewayGate
from lp_gauge import LPGauge
from dev_node import DevNode
from driveway_sensor import DrivewaySensor

logger = logging.getLogger(__name__)


def on_disconnect(userdata, rc, properties):
    logger.warning("mqtt disconnected. terminating")
    global time_to_die
    time_to_die = True


def on_connect(mqttc, obj, flags, rc):
    logger.warning("mqtt connected, rc: " + str(rc))


class LoRaNetBridge(entities.LoRaNode):
    def __init__(self, id, radio, mqtt_client):
        super().__init__("LoRaNet Bridge", id, radio, mqtt_client)
        self.device_config = entities.DeviceConfig(f"{self.name}")
        self.uptime = entities.Sensor(f"{self.name} uptime", self.device_config, mqtt_client, "s")
        self.start_time = datetime.datetime.now()

    def update_state(self):
        self.uptime.publish_state((datetime.datetime.now() - self.start_time).seconds)


def main():
    parser = argparse.ArgumentParser(description="loranet bridge")
    parser.add_argument('-v', "--verbose", action="count", help="Increase verbosity of outut", default=0)
    args = parser.parse_args()

    if   args.verbose == 0: level = logging.WARNING
    elif args.verbose == 1: level = logging.INFO
    elif args.verbose > 1:  level = logging.DEBUG

    # Eventually may put argparse/config file processing here.
    logging.basicConfig(format="%(asctime)s.%(msecs)03d %(threadName)s %(levelname)s %(name)s: %(message)s",
        level=level,
        stream=sys.stdout,
        datefmt="%H:%M:%S")

    radio = LoRaBase()

    mqtt_username = os.environ.get("mqtt_username", "")
    mqtt_password = os.environ.get("mqtt_password", "")

    global mqtt_client
    mqtt_client = mqtt.Client()
    mqtt_client.on_disconnect = on_disconnect
    mqtt_client.on_connect = on_connect

    if len(mqtt_username) and len(mqtt_password):
        mqtt_client.username_pw_set(mqtt_username, mqtt_password)
    else:
        logger.error("No username/password specified")
        return

    mqtt_client.connect("duckling.groot")

    mqtt_client.loop_start()

    loranet_bridge = LoRaNetBridge(0, radio, mqtt_client)
    driveway_gate = DrivewayGate("Driveway Gate", 2, radio, mqtt_client)
    lp_gauge = LPGauge("LP Gauge", 3, radio, mqtt_client)
    #dev_node = DevNode("Dev Node", 4, radio, mqtt_client)
    driveway_sensor = DrivewaySensor("Driveway Sensor", 5, radio, mqtt_client)

    global time_to_die
    time_to_die = False
    while not time_to_die:
        msg = radio.rx()
        if msg is not None:
            sender = msg[1]
            packet = msg[4:].decode()
            logger.info(f"Rx from:{sender},rssi:{radio.rfm9x.rssi},snr:{radio.rfm9x.snr},msg:{packet}")
            #dev_node.update_state(sender, packet)
            driveway_gate.update_state(sender, packet)
            lp_gauge.update_state(sender, packet)
            driveway_sensor.update_state(sender, packet)
        else:
            time.sleep(0.01)

        #dev_node.request_state()
        driveway_gate.request_state()
        driveway_sensor.checkin()
    sys.exit(-1)


if __name__ == "__main__":
    main()