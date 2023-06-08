# LoraNet bridge

## Classes
    LoRaBase : Interface to the base station LoRa radio; no threads tx/rx calls can take hundreads of ms
    DeviceConfig: data to describe a home assistant/mqtt device
    EntityConfig: data to describe a home assistant/mqtt entity (a device can have multiple entities)
    EntityBase: base class for the functionality of a single entity
    LoRaNode:

define callbacks for objects that need

## raspberry pi config for bridge
sudo apt install python3-pip python3-venv
sudo pip3 install --upgrade setuptools
sudo pip3 install --upgrade adafruit-python-shell
wget https://raw.githubusercontent.com/adafruit/Raspberry-Pi-Installer-Scripts/master/raspi-blinka.py
sudo python3 raspi-blinka.py
reboot

## python environment
```
cd loranet/bridge
python3 -m venv .venv --system-site-packages
. .venv/bin/activate
pip install -r requirements.txt


### systemd unit

A user systemd unit is defined in
`bridge/loranet-bridge.service`

Install it doing the following:
```bash
loginctl enable-linger $USER
mkdir -p ~/.config/systemd/user
cp bridge/loranet-bridge.service ~/.config/systemd/user
systemctl --user enable loranet-bridge.service
systemctl --user start loranet-bridge.service
```

To reload a changed service
```bash
systemctl --user disable loranet-bridge.service
systemctl --user daemon-reload
```

This service reads $HOME/git/loranet/bridge/secrets to define the folling environment vars:
```
mqtt_hostname
mqtt_username
mqtt_password
```
