# GateManager 
## Hardware
### Renogy Rover
#### RJ-12 Jack 
Talks modbus protocol. See https://www.dropbox.com/s/8itoxh8c3uf4ncq/ROVER%20MODBUS.DOCX?dl=0 for the specs
See https://github.com/corbinbs/solarshed for some python code that talks to one

Using a standard RJ-12 cable (blue,yellow,green,red,black,white)
blue is the rover's RxD
yellow is the rover's TxD
green is the rover's ground
black, is rover +12V

### Mule gate controller

### Adafruit LoRa feather
#### Connectors / Nets
J1-1 (PCB1)level converter ground; to rover modbus ground (pin 3 on jack) (grey on RJ12 cable)
J1-2 (PCB2) Level converter "ttl RxD"; to rover modbus TxD (pin 2 on jack) (green on RJ12 cable)
J1-3 (PCB3) Level converter "ttl TxD"; to rover modbus RxD (pin 1 rover jack) (blue on JR12 cable)

J2-1 (PCB6), ground
J2-2 (PCB7), feather pin 21, relay 1, (gate safe) 
J2-3 (PCB8), feather pin 22, relay 2, (gate open)
J2-4 (PCB9), feather pin 23, relay 3, (poe power enable)
J2-5 (PCB10), ground
J2-6  (PCB11), feather 20, relay 4, (gate enable)
J2-7  (PCB12), feather 19, tbd
J2-8  (PCB13), feather 18 , tbd

J3-1 (PCB22) ground
J3-2 (PCB22), 5VDC, to feather USB (VBUS), to 5VDC converter
J3-3 (PCB22), Vbat-sla, to 33k/10k resistive divider (3.3:1), to feather pin A11

### ttl relay board

