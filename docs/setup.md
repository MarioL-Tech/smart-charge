# Setup Guide

How to set up the EVSE-Control project from scratch: Raspberry Pi, ESP32, wiring, and the first connection test.

## 1. Requirements

**Hardware**

- Raspberry Pi (3 or 4) with Raspberry Pi OS (Bookworm) installed
- ESP32 DevKit (e.g. WROOM-32)
- 3 jumper wires (female-female)
- microSD card for the Pi

**Software**

- Raspberry Pi OS with WiFi + SSH enabled
- Mosquitto (MQTT broker) — needed for the MQTT phase
- `pyserial` on the Pi (Python serial library)
- PlatformIO + VS Code for flashing the ESP32

## 2. Raspberry Pi preparation

### WiFi

```bash
nmcli device wifi list
nmcli device wifi connect "SSID" password "PASSWORD"
```

### SSH

```bash
sudo systemctl enable --now ssh
```

### Keyboard layout (optional, German example)

```bash
sudo raspi-config
```

-> Localisation Options -> Keyboard -> German

### Enable the serial port (required!)

```bash
sudo raspi-config
```

-> Interface Options -> Serial Port

- "login shell accessible over serial?" -> **No**
- "serial port hardware enabled?" -> **Yes**

Then reboot:

```bash
sudo reboot
```

Verify the device exists:

```bash
ls -l /dev/serial*
```

### Python dependencies

```bash
sudo apt install python3-serial
```

(On Bookworm, bare `pip install` is blocked by PEP 668 — use apt or a venv.)

## 3. Wiring (ESP32 <-> Raspberry Pi)

Both sides use 3.3V logic — **no level shifter needed**.

| Raspberry Pi (40-pin header) | ESP32 DevKit |
|---|---|
| GPIO 14 / TXD (pin 8) | GPIO 16 (Serial2 RX) |
| GPIO 15 / RXD (pin 10) | GPIO 17 (Serial2 TX) |
| GND (pin 6) | GND |

**Warning:** never connect 5V to the ESP32 GPIOs.

## 4. Flash the ESP32

Open the `esp32/` folder in VS Code (PlatformIO extension installed), then:

```bash
pio run -t upload
```

The sketch sends a counter line every second over Serial2 (GPIO 16/17, 115200 baud) and echoes everything it receives.

## 5. Run the connection test

On the Pi, from the `rasppi/` directory:

```bash
python3 src/main.py
```

Expected output:

```text
EVSE connection test: listening on /dev/serial0 @ 115200 baud
TX -> ESP32: PING
RX <- ESP32: ECHO:PING
RX <- ESP32: EVSE_TEST:0
RX <- ESP32: EVSE_TEST:1
```

- `EVSE_TEST` lines = ESP32 -> Pi direction works
- `ECHO:PING` = Pi -> ESP32 direction works
- Login/boot text (`raspberrypi login:`, `Password:`) in the output means the serial console is still enabled — repeat step 2 (Serial Port settings) and reboot.

## 6. MQTT (next step)

Install the broker on the Pi:

```bash
sudo apt install mosquitto mosquitto-clients
```

Test locally:

```bash
mosquitto_sub -h localhost -t "evse/test"   # terminal 1
mosquitto_pub -h localhost -t "evse/test" -m "hello"   # terminal 2
```

The ESP32 will connect via WiFi to the broker on port 1883 using the Pi's IP (`hostname -I`).
