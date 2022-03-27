#!/usr/bin/env python3
# -*- coding: utf-8 -*-
""" A bridge between LoRaNet devices and MQTT. The local radio is an Adafruit LoRa bonnet for the raspberry pi
"""

import datetime
import logging
import os
import time

from lorabase import LoRaBase
import entities

logger = logging.getLogger(__name__)

def main():
    # Eventually may put argparse/config file processing here.
    logging.basicConfig(format="%(asctime)s.%(msecs)03d %(threadName)s: %(message)s",
        level=logging.DEBUG,
        datefmt="%H:%M:%S")

    radio = LoRaBase()

    logger.info("Starting")
    while True:
        logger.info("================")
        radio.tx(2, "foo")
        msg = radio.rx(2, 1.0)
        logger.info(f"Rx:{msg}")
        time.sleep(5);


if __name__ == "__main__":
    main()