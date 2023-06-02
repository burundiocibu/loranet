# Hardware
## Renogy Rover/Wanderer
### RJ-12 Jack 
Talks modbus protocol. See https://www.dropbox.com/s/8itoxh8c3uf4ncq/ROVER%20MODBUS.DOCX?dl=0 for the specs

Using a standard RJ-12 cable (blue,yellow,green,red,black,white)
blue is the rover's RxD
yellow is the rover's TxD
green is the rover's ground
black, is rover +12V

### Main proto board overview
This is a prototyping board with DIN mounting brackets. On this board are the following

* Heltec ESP32 LoRa module V3
   * https://heltec.org/project/wifi-lora-32-v3/
   * datasheet: https://resource.heltec.cn/search/
   * SX1262 LoRa radio 
   * 0.96 128x64 OLED display, On I2C bus, U6, 
   * CP2102 USB to serial
   * schematics: https://resource.heltec.cn/download/WiFi_LoRa32_V3/HTIT-WB32LA(F)_V3_Schematic_Diagram.pdf
   * docs: https://docs.heltec.org/en/node/esp32/index.html
* Generic RS232-TTL level converter to talk to the renology charge contoller 
* Cytron brushed dc motor driver https://makermotor.com/pn00218-cyt2-cytron-13a-dc-motor-driver-md10c/
   https://makermotor.com/content/cytron/pn00218-cyt2/MD10C%20Rev2.0%20User%27s%20Manual.pdf
* dual comparitor to generate interrupts from the encoder line
* Keystone Heddolf  P294-KB Universal Receiver
   https://www.keystoneheddolf.com/our-products
* Chtoocy Wireless Driverway alarm.
   https://manuals.plus/chtoocy/ch-mr2022-solar-wireless-driveway-alarm-manual#axzz82ODpOYUY

### Modules

#### Heltec ESP32 LoRa moduel
ESP32-S3FN8, 32bit LX7, 240 MHz, 8MB flash, 320KB ram
https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/index.html

Pin numbers are arduino pin numbers or silkscreen

Some libs to investigated:
* Heltec ESP32 Dev-Boards
* Heltec_esp32_display
* oled ssd1306
* gxepd
* RadioLib

#### Protoboard
Right
bb  | pin   | fcn    |
----|-----  | ----   |
f1  | J3.1  | gnd    |
f2  | J3.2  | 3v3    |
f3  | J3.3  | 3v3    |
f4  | J3.4  | gpio37 | ADC_Ctrl w pullup
f5  | J3.5  | gpio46 | Output, MD10C_PWM
f6  | J3.6  | gpio45 | Output, MD10C_DIR
f7  | J3.7  | gpio42 | Output, RELAY1_IN
f8  | J3.8  | gpio41 | 
f9  | J3.9  | gpio40 | 
f10 | J3.10 | gpio39 | 
f11 | J3.11 | gpio38 | Input, REMOTE_RECEIVER, with 5k pull up
f12 | J3.12 | gpio1  | VBAT_read 
f13 | J3.13 | gpio2  | 
f14 | J3.14 | gpio3  |
f15 | J3.15 | gpio4  |
f16 | J3.16 | gpio5  | 
f17 | J3.17 | gpio6  | Input, Internal pullup enabled, ENCODER_PULSE
f18 | J3.18 | gpio7  | Input, Internal pullup enabled, ENCODER_LIMIT

---
Left
bb  | pin   | fcn    | 
----|-----  | ----   | 
e1  | J2.1  | gnd    |
e2  | J2.2  | 5v     |
e3  | J2.3  | Ve     |
e4  | J2.4  | Ve     |
e5  | J2.5  | U0RxD  | CP2102.RxD
e6  | J2.6  | UoTxD  | CP2102.TxD
e7  | J2.7  | rst    | RST_SW
e8  | J2.8  | gpio0  | USER_SW
e9  | J2.9  | gpio36 | Vext_Ctrl w pullup
e10 | J2.10 | gpio35 | LED_Write
e11 | J2.11 | gpio34 | 
e12 | J2.12 | gpio33 | Renogy TTL-RxD
e13 | J2.13 | gpio47 | Renogy TTL-TxD
e14 | J2.14 | gpio48 |
e15 | J2.15 | gpio26 |
e16 | J2.16 | gpio21 | OLED_RST
e17 | J2.17 | gpio20 |
e18 | J2.18 | gpio19 |

Internal Connections
esp32  | fcn
-----  | ---
GPIO8  | LoRa_NSS
GPIO9  | LoRa_SCK
GPIO10 | LoRa_MOSI
GPIO11 | LoRa_MISO
GPIO12 | LoRa_RST
GPIO13 | LoRa_BUSY
GPIO17 | OLED_SDA
GPIO18 | OLED_SCL

| Relay   | Connections
|---------|------------
| 5V      | 5VDC
| Gnd     | Gnd
| D1      | ESP32 gpio2, RELAY1_IN
| NO      | Act Yel
| COM     | VLOAD

| MD10C   | Connections
|---------|------------
| PWR+    | VLOAD
| PWR-    | Gnd
| A       | Act Red
| B       | Act Blk
| PWM     | ESP32 
| DIR     | ESP32
| Gnd     | Gnd

| TTL     | Connection
|---------|------------
| RxD     | ESP32
| TxD     | ESP32
| Gnd     | Gnd
| Vcc     | 5VDC

| Terminals | Connections
|-----------|------------
| 12VDC     | Charge controller Vload +, PWB 12VDC buss
| Gnd       | Charge controller Vload -, PWB Gnd buss, B2, B3
| Act Wht   | Encoder signal, PWB B70, Acutator white
| Act Grn   | Encoder ground
| Act Red   | Motor +, TB67H420 A+, Actuator Red
| Act Blk   | Motor -, TB67H420 A-, Actuator Black
| Act Blu   | Lock -, Gnd
| Act Yel   | Lock +, Relay NO

#### Gate actuator
Actuator is a MM371W, made by MightyMule.
Linear actuator has a 12VDC brushed motor. Motor has an inrush current of 16A and a no-load current of 1.6A.
Motor has what appears to be a hall effect sensor board that outputs three levels.
2 V when limit switch is triggered
toggles between 3 and 4.2V when motor is running
pulses are about 3 ms apart when motor is at full speed. closer starting/stopping
pulses are about 9.2 ms wide when motor is running at full speed.
With a 1k pullup, pulses are 1.84 to 2.76V

#### Encoder conditioning
EncoderIN is w 1 k pullup to +5 & 50nF cap to ground through comparitor set to 2V, e15
Encoder2 is above through a comparitor set to .8V

ENCODER_PULSE is pulsed high with shaft rotation, 9ms pulses at "full" speed
ENCODER_LIMIT is set high when limit switch is hit
