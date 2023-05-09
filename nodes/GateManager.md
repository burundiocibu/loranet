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
* Pololu brushed dc motor driver https://www.pololu.com/product/2999/resources
   * TB67H420 motor driver: https://toshiba.semicon-storage.com/info/TB67H420FTG_datasheet_en_20201016.pdf?did=59110&prodName=TB67H420FTG
   * HBMODE is strapped hight to run both  channels in parallel
   * all input is on PWMA, INA1, and INA2. PWMB, INB1, and INB2 are ignored
   * Connect to feather: PWMA, INA1, INA2, LO1, LO2

### Modules

#### Feather 32u4 with RFM9x
ATmega32u4 @ 8 MHz, 32k flash, 2k ram

Pin numbers are arduino pin numbers or silkscreen

#### Mainboard

Numbers are columns, 74 is far right

Letters are rows, A row with connector blocks

Feather numbers are silkscreen

| PWB  | Feather | Connections
|------|---------|------------
| C59  | 2/SDA   | Encoder signal, w 5 k pullup to +5 * "filter", PWB B70
| C60  | 3/SCL   | TB67H420 LO2, PWB B58
| C61  | 5       | TB67H420 PWMA, TIMER3A, PWB B53
| C62  | 6/A7    | TB67H420 LO1, PWB B57
| C63  | 9/A9    | Feather Vbat via 1:1 divider
| C64  | 10/A10  | TB67H420 INA1, PWB B51
| C65  | 11      | TB67H420 INA2, PWB B52
| C66  | 12/A11  | System Vload, via 33k:10k divider, PWB B66
| C67  | 13      | Feather LED
| C68  | Vbus    | 5VDC buss
| C69  | EN      |
| C70  | Vbat    | Feather vbat
|------|---------|-------------
| D59  | DIO1    | 
| D60  | 1/TxD1  | TTL-RxD
| D61  | 0/RxD1  | TTL-TxD, PWB C26
| D62  | MISO    | RMF9x SPI
| D63  | MOSI    | RMF9x SPI
| D64  | SCK     | RMF9x SPI
| D65  | A5      |
| D66  | A4      |
| D67  | A3      | Driveway remote sensor in
| D68  | A2      | Wireless remote receiver in, PWB A55
| D69  | A1      | To Relay in, Relay D1, for gate lock, PWB C53
| D70  | A0      | Encoder signal, PWB C59
| D71  | GND     | Gnd, PWB B2
| D72  | AREF    |
| D73  | 3.3V    | 
| D74  | RST     |

| PWB  | Relay   | Connections
|------|---------|------------
| C48  | 5V      | 5VDC buss
| C49  | Gnd     | Gnd
| C53  | D1      | Feather A1, PWB D69
|      | NO      | Act Yel, PWB A63
|      | COM     | Vload buss

| PWB  | TB67H420| Connections
|------|---------|------------
| B48  | VM      | 
| B49  | Gnd     | Gnd
| B50  | Vcc     | 
| B51  | INA1    | Feather 10, PWB C64
| B52  | INA2    | Feather 11, PWB C65
| B53  | PWMA    | Feather 3, PWB C60
| B54  | INB1    | 
| B55  | INB2    | 
| B56  | PWMB    | 
| B57  | LO1     | Feather 6, PWB C62
| B58  | LO2     | Feather 5, PWB C61
| B59  | VrefA   |
|      | A+      | Act Red, PWB A65
|      | A-      | Act Blk, PWB A63
|      | Vin     | Vload buss
|      | Gnd     | Gnd buss

| PWB  | TTL     | Connections
|------|---------|------------
| C26  | RxD     | Feather 1, PWB D60
| C27  | TxD     | Feather 0, PWB D61
| C28  | Gnd     | Gnd
| C29  | Vcc     | 5VDC buss

| PWB  | Terminals | Connections
|------|-----------|------------
| A74  | 12VDC     | Charge controller Vload +, PWB 12VDC buss
| A72  | Gnd       | Charge controller Vload -, PWB Gnd buss, B2, B3
| A70  | Act Wht   | Encoder signal, PWB B70, Acutator white
| A67  | Act Grn   | Encoder ground
| A65  | Act Red   | Motor +, TB67H420 A+, Actuator Red
| A63  | Act Blk   | Motor -, TB67H420 A-, Actuator Black
| A60  | Act Blu   | Lock -, Gnd
| A58  | Act Yel   | Lock +, Relay NO
| A55  | Rcvr      | Remote receiver signal, Feather A2, PWB D68
| A53  | Rcvr Gnd  | Remote receiver Gnd, Gnd, Gnd buss

