#!/usr/bin/env python3
# -*- coding: utf-8 -*-
""" Some window dressing on the adafruit rfm9x library...
"""

import logging
import threading
import time
from typing import Optional

import busio
from digitalio import DigitalInOut, Direction, Pull
import board
import adafruit_ssd1306
from adafruit_rfm9x import RFM9x
#from jcl_rfm9x import RFM9x


# Create the I2C interface.
i2c = busio.I2C(board.SCL, board.SDA)

# 128x32 OLED Display
reset_pin = DigitalInOut(board.D4)
display = adafruit_ssd1306.SSD1306_I2C(128, 32, i2c, reset=reset_pin)
# Clear the display.
display.fill(0)
display.text('LoRaNet', 0, 0, 1)
display.show()

logger = logging.getLogger(__name__)


class LoRaBase():
    def __init__(self):
        # Configure LoRa Radio, https://circuitpython.readthedocs.io/projects/rfm9x/en/latest/
        # https://github.com/adafruit/Adafruit_CircuitPython_RFM9x
        # http://www.airspayce.com/mikem/arduino/RadioHead/classRH__RF95.html
        # https://learn.adafruit.com/adafruit-radio-bonnets/pinouts
        # https://www.hoperf.com/data/upload/portal/20190801/RFM95W-V2.0.pdf

        CS = DigitalInOut(board.CE1)
        RESET = DigitalInOut(board.D25)
        #IRQ  = board.DIO0 # GPIO22

        self.spi = busio.SPI(board.SCK, MOSI=board.MOSI, MISO=board.MISO)
        self.rfm9x = RFM9x(self.spi, CS, RESET, 915.0) # 915 Mhz
        self.tx_power = 20
        self.rfm9x.tx_power = self.tx_power
        self.rfm9x.node =  0
        self.rfm9x.flags = 0 # default is 0
        self.rfm9x.preamble_length = 12
        self.rfm9x_lock = threading.Lock()
        self.tx_time = 0
        self.rx_time = 0
        self.reply_timeout = 2 # how long to wait for a response
        

    def tx(self, dest, msg:str) -> bool:
        with self.rfm9x_lock:
            t0 = time.perf_counter()
            self.rfm9x.destination = dest #  default is 255 (broadcast)
            if not self.rfm9x.send(bytes(msg, "utf-8"), keep_listening=True):
                logger.debug("tx err")
                return False
            self.rfm9x.listen()
            self.tx_time = time.monotonic()
            dt = round(1000 * (time.perf_counter() - t0), 1)
            logger.info(f"Tx to:{dest}, dt:{dt}, msg:{msg}")
            return True


    def rx(self):
        with self.rfm9x_lock:
            # If receive is called and the the message times out in the middle
            # of the message, it will idle the radio stopping receiption of a message
            if self.rfm9x.rx_done():
                packet = self.rfm9x.receive(with_header=True, keep_listening = True)
                if packet is None:
                    return None
                else:
                    self.rx_time = time.monotonic()
                    return packet
            else:
                return None;


    def free_to_send(self) -> bool:
        dt = time.monotonic() - self.tx_time
        return self.rx_time > self.tx_time or dt > self.reply_timeout


    # note this has to be seperate since it can't ack and broadcast
    def broadcast(self, msg:str) -> None:
        self.rfm9x.destination = 255 #  default is 255 (broadcast)
        self.rfm9x.send(bytes(msg, "utf-8"))


    def decode_msg(self, msg:str) -> dict:
        d = {}
        for s in msg.split(","):
            if len(s):
                n,v = s.split(':')
                d[n] = v
        return d
