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

* Adafruit LoRa feather module with an ATMega32u4 and RFM9x LoRa radio https://www.adafruit.com/product/3078
* Generic RS232-TTL level converter to talk to the renology charge contoller 
* Cytron brushed dc motor driver https://makermotor.com/pn00218-cyt2-cytron-13a-dc-motor-driver-md10c/
   https://makermotor.com/content/cytron/pn00218-cyt2/MD10C%20Rev2.0%20User%27s%20Manual.pdf
* dual comparitor to generate interrupts from the encoder line
* Keystone Heddolf  P294-KB Universal Receiver
   https://www.keystoneheddolf.com/our-products
* Chtoocy Wireless Driverway alarm.
   https://manuals.plus/chtoocy/ch-mr2022-solar-wireless-driveway-alarm-manual#axzz82ODpOYUY

### Modules

#### Feather 32u4 with RFM9x
ATmega32u4 @ 8 MHz, 32k flash, 2k ram

Pin numbers are arduino pin numbers or silkscreen

#### Mainboard

Feather numbers are silkscreen

| Feather | Connections
|---------|------------
| 2/SDA   | Encoder pulse signal, e15
| 3/SCL   | This appears to be used by the RadioHead drivers
| 5       | MD10C_PWM, TIMER3A, e17
| 6/A7    | 
| 9/A9    | Feather Vbat via 1:1 divider
| 10/A10  | MD10C_DIR, e20
| 11      | Encoder limit signal, PCINT7 PB7, e21
| 12/A11  | System Vload, via 33k:10k divider, e22
| 13      | Feather LED
| Vbus    | 5VDC buss
| EN      |
| Vbat    | Feather vbat
|---------|-------------
| DIO1    | 
| TxD1    | TTL-RxD f15 
| RxD1    | TTL-TxD f16
| MISO    | RMF9x SPI
| MOSI    | RMF9x SPI
| SCK     | RMF9x SPI
| A5      |
| A4      |
| A3      | Driveway remote sensor in, e23
| A2      | from Heddolf Receiver output, e24
| A1      | To Relay IN, for gate lock, e25
| A0      | 
| GND     | Gnd
| AREF    |
| 3.3V    | 
| RST     |

| Relay   | Connections
|---------|------------
| 5V      | 5VDC buss
| Gnd     | Gnd
| D1      | Feather A1, PWB D69
| NO      | Act Yel, PWB A63
| COM     | Vload buss

| MD10C   | Connections
|---------|------------
| PWR+    | 
| PWR-    | Gnd
| A       | Act Red, PWB A65
| B       | Act Blk, PWB A63
| PWM     | Feather 5
| DIR     | Feather 10
| Gnd     | Feather Gnd

| TTL     | Connection
|---------|------------
| RxD     | Feather TxD, e1
| TxD     | Feather RxD, e2
| Gnd     | Gnd, a3
| Vcc     | 5VDC,a4


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
| Rcvr      | Remote receiver signal, Feather A2, PWB D68
| Rcvr Gnd  | Remote receiver Gnd, Gnd, Gnd buss

#### Remote recevier
Mighty Mule AQ201-NB
Takes +5 to drive
uses a Hope RF CMT2210LH OOK receiver

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
Encoder1 is w 1 k pullup to +5 & 50nF cap to ground through comparitor set to 2V, e15
Encoder2 is above through a comparitor set to .8V

Encoder 1 is pulsed high with shaft rotation
Encoder 2 is pulsed high when limit switch is hit
