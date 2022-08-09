#!/usr/bin/env python3
# -*- coding: utf-8 -*-
""" Some window dressing on the adafruit rfm9x library...
"""

import logging
import threading
import time

import busio
from digitalio import DigitalInOut, Direction, Pull
import board
import adafruit_ssd1306
import adafruit_rfm9x


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
        self.rfm9x = adafruit_rfm9x.RFM9x(self.spi, CS, RESET, 915.0) # 915 Mhz
        self.tx_power = 23
        self.rfm9x.tx_power = self.tx_power
        self.rfm9x.node = 0
        self.rfm9x.flags = 0 # default is 0
        self.tx_stats = {'count':0, 'err':0 }
        self.rx_stats = {'count':0, 'err':0 }
        self.rfm9x_lock = threading.Lock()
        self.msg = None # most recent received message

    def print_stats(self, msg:str) -> None:
        logger.debug(f"{msg}  tx:{self.tx_stats}  rx:{self.rx_stats}")

    def tx(self, dest, msg:str) -> bool:
        with self.rfm9x_lock:
            logger.debug("Start Tx")
            t0 = time.perf_counter()
            self.rfm9x.destination = dest #  default is 255 (broadcast)
            self.tx_stats['count'] += 1
            if not self.rfm9x.send_with_ack(bytes(msg, "utf-8")):
                self.tx_stats['err'] += 1
                self.print_stats("noack")
                return False
            self.rfm9x.listen()
            dt = round(1000 * (time.perf_counter() - t0), 1)
            self.tx_stats['rtt'] = dt
            self.tx_stats['rssi'] = self.rfm9x.last_rssi
            self.tx_stats['snr'] = self.rfm9x.last_snr
            self.tx_stats['txpwr'] = self.rfm9x.tx_power
            self.print_stats(f"tx ok")
            return True

    def rx(self, sender, timeout:float = 1.0):
        with self.rfm9x_lock:
            logger.debug("Start Rx")
            t0 = time.perf_counter()
            self.rfm9x.receive_timeout = timeout # seconds
            self.rfm9x.destination = sender
            packet = self.rfm9x.receive(with_header=True, with_ack=True)
            self.rx_stats['rtt'] = round(1000 * (time.perf_counter() - t0),0)
            sender = packet[1]
            self.msg = packet[4:]
            if packet is None:
                self.rx_stats['err'] += 1
                self.print_stats('timeout')
                return None
            self.rx_stats['count'] += 1
            logger.debug(f"Rx from:{sender}, msg:{self.msg}")
            return packet

    # note this has to be seperate since it can't ack and broadcast
    def broadcast(self, msg:str) -> None:
        self.rfm9x.destination = 255 #  default is 255 (broadcast)
        self.rfm9x.send(bytes(msg, "utf-8"))

    # note this has to be seperate since it can't ack and broadcast
    def listen(self, timeout:float = 0.5):
        with self.rfm9x_lock:
            t0 = time.perf_counter()
            self.rfm9x.listen()
            self.rfm9x.destination = 0 #  default is 255 (broadcast)
            self.rfm9x.receive_timeout = timeout # seconds
            packet = self.rfm9x.receive(with_header=True, with_ack=True, keep_listening=True)
            dt = round(1000 * (time.perf_counter() - t0), 0)
            if packet is not None:
                return int(packet[1]), packet[4:]
            else:
                return None, None

    def decode_msg(self, msg:str) -> dict:
        d = {}
        for s in msg.decode().split(","):
            if len(s):
                n,v = s.split(':')
                d[n] = v
        return d
