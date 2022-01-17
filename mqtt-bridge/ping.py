#!/usr/bin/env python3

import paho.mqtt.client as mqtt

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
display.text('LoRa radio', 0, 0, 1)
display.show()

# Configure LoRa Radio, https://circuitpython.readthedocs.io/projects/rfm9x/en/latest/
# https://github.com/adafruit/Adafruit_CircuitPython_RFM9x
CS = DigitalInOut(board.CE1)
RESET = DigitalInOut(board.D25)
spi = busio.SPI(board.SCK, MOSI=board.MOSI, MISO=board.MISO)
rfm9x = adafruit_rfm9x.RFM9x(spi, CS, RESET, 915.0) # 915 Mhz
tx_power = 23
rfm9x.tx_power = tx_power


rfm9x.destination = 2 #  default is 255 (broadcast) 
rfm9x.node = 1 # RH calls this source, default is 255 (broadcast)
rfm9x.flags = 0 # default is 0


tx_stats = {'count':0, 'err':0 }
rx_stats = {'count':0, 'err':0 }
node = {}

def print_stats(msg:str) -> None:
    print(f"{msg:10}  tx:{tx_stats}  rx:{rx_stats}  node:{node}")


def tx(msg:str) -> bool:
    t0 = time.perf_counter()
    tx_stats['count'] += 1
    if not rfm9x.send_with_ack(bytes(msg, "utf-8")):
        tx_stats['err'] += 1
        print_stats("noack")
        return False
    tx_stats['rtt'] = round(1000 * (time.perf_counter() - t0), 1)
    tx_stats['rssi'] = rfm9x.last_rssi
    tx_stats['snr'] = rfm9x.last_snr
    tx_stats['txpwr'] = rfm9x.tx_power
    if True or tx_stats['count'] % 10 == 0:
        print_stats("ok")
    return True
    

def rx(timeout:float = 0.5):
    t0 = time.perf_counter()
    rfm9x.receive_timeout = timeout # seconds
    packet = rfm9x.receive(with_ack=True)
    rx_stats['rtt'] = round(1000 * (time.perf_counter() - t0),)
    if packet is None:
        rx_stats['err'] += 1
        print_stats('timeout')
    else:
        rx_stats['count'] += 1
    return packet


def decode_msg(msg:str) -> dict:
    d = {}
    for s in msg.decode().split(","):
        if len(s):
            n,v = s.split(':')
            d[n] = v
    return d


# messages:
# S      return status message
# P:pwr  set tx power
# 
while True:
    if not tx("S"):
        continue

    packet = rx(0.75)
    if packet is not None:
        display.fill(0)
        display.text(f"RSSI: {rfm9x.last_rssi} dB", 0, 10, 1)
        display.text(f"SNR: {rfm9x.last_snr} dB", 0, 20, 1)
        node = decode_msg(packet)
        display.show()

        pwr = int(node['txpwr'])
        if pwr != tx_power:
            tx(f"P:{tx_power}")

    time.sleep(1)
