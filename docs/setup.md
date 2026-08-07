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

Both sides use 3.3V logic — **no level shifter needed**. The detailed pin reference lives in [docs/pin-connection.md](pin-connection.md).

| Raspberry Pi (40-pin header) | ESP32 DevKit | Signal |
|---|---|---|
| GPIO 14 / TXD (pin 8) | GPIO 16 (Serial2 RX) | Pi TX → ESP32 RX |
| GPIO 15 / RXD (pin 10) | GPIO 17 (Serial2 TX) | Pi RX ← ESP32 TX |
| GND (pin 6) | GND | Common ground (GND ↔ GND) |

**Warnings:**

- TX ↔ RX is **crossover** — never connect TX to TX.
- **GND ↔ GND is mandatory** (common ground for the UART signal).
- Never connect 5V to the ESP32 GPIOs.

### MFRC522 RFID reader (optional, manual override)

| MFRC522 | ESP32 DevKit |
|---|---|
| SDA/SS | GPIO 5 |
| SCK | GPIO 18 |
| MOSI | GPIO 23 |
| MISO | GPIO 19 |
| RST | GPIO 22 |
| 3.3V | 3.3V |
| GND | GND |

The RFID reader is the **manual override for the anti-theft lock**: tapping a
card locks (servo 90°), tapping again unlocks (servo 0°). The lock is
**independent** of the charging state. See `docs/pin-connection.md` for the
full pin reference and `docs/uart-protocol.md` for the ESP32 ↔ Pi protocol.

### Anti-theft servo (model servo)

| Servo (e.g. SG90) | ESP32 DevKit / PSU |
|---|---|
| Signal (orange) | GPIO 13 |
| VCC (red) | 5V (external or Pi 5V pin) |
| GND (brown) | GND (common with ESP32) |

The servo acts as the anti-theft lock, toggled by **RFID taps** (manual
override, independent of charging): first tap locks (**90°**), next tap
unlocks (**0°**). At boot the lock starts **released (0°)**.
**Do not power the servo from the ESP32 3V3 pin** — use a 5V supply with a
common ground (see `docs/pin-connection.md`).

## 4. Flash the ESP32

Open the `esp32/` folder in VS Code (PlatformIO extension installed), then:

```bash
pio run -t upload
```

The firmware initializes the MFRC522 RFID reader and the anti-theft servo,
starts the UART link (GPIO 16/17, 115200 baud) and reports its state at boot
(see §5 for the expected output).

## 5. Run the connection test

On the Pi, from the `rasppi/` directory:

```bash
python3 src/main.py
```

This starts the UART bridge: it prints everything the ESP32 sends and lets
you send commands (`on` / `off` / `status` or raw protocol lines).

**Expected boot output** (as soon as the ESP32 is powered/reset):

```text
EVSE UART bridge: listening on /dev/serial0 @ 115200 baud
RX <- ESP32: EVSE:STATUS:CHARGING:OFF:SRC:boot
RX <- ESP32: EVSE:STATUS:ANTITHEFT:INACTIVE:SRC:boot
```

On the ESP32's **USB serial monitor** (115200 baud) the same boot lines
appear, plus the RFID diagnostic:

```text
MFRC522 firmware version: 0x82
EVSE RFID controller started
EVSE:STATUS:CHARGING:OFF:SRC:boot
EVSE:STATUS:ANTITHEFT:INACTIVE:SRC:boot
```

**Interpreting the version line:**

- `0x91` / `0x92` = MFRC522 chip detected
- `0x82` = **PN512** chip (common on cheap "MFRC522" modules) — also OK,
  register-compatible, reads ISO 14443A cards
- `0x00` / `0xFF` = **wiring/power problem** — the ESP32 cannot talk to the
  reader (check SS/SCK/MOSI/MISO/RST, 3.3V, GND)

**Anti-theft toggle test** (with a 13.56 MHz card, e.g. the white Mifare
cards/key fobs that ship with the module):

```text
RX <- ESP32: EVSE:RFID:CARD:UID:AB:CD:EF:12
RX <- ESP32: EVSE:STATUS:ANTITHEFT:ACTIVE:SRC:rfid     # servo → 90°
# next tap:
RX <- ESP32: EVSE:RFID:CARD:UID:AB:CD:EF:12
RX <- ESP32: EVSE:STATUS:ANTITHEFT:INACTIVE:SRC:rfid   # servo → 0°
```

**Troubleshooting:**

- `raspberrypi login:` / `Password:` text in the bridge output → the serial
  console is still enabled — repeat step 2 (Serial Port settings) and reboot.
- No boot lines at all on the Pi → UART wiring (TX/RX crossover, GND) or the
  Pi serial config; check with the USB serial monitor whether the ESP32 boots.
- Boot lines appear but no RFID reaction → the reader is alive (check the
  version line); verify the card is **13.56 MHz** (Mifare/NTAG — MFRC522/PN512
  cannot read 125 kHz cards) and hold it flat and still on the antenna.
- No boot lines even on the USB monitor → flash/upload problem.

## 6. Wallbox interface (ABB Terra AC, Modbus RTU)

The charging station is controlled by the **Raspberry Pi** over Modbus RTU
(RS-485) using a USB-RS485 adapter — no ESP32 pins involved.

**Hardware**

- USB-RS485 adapter (e.g. CH340/CP2102-based, 3.3V/5V tolerant)
- The wallbox's RS-485 A/B terminals (see Terra AC Installation Manual)

**Wiring**

| USB-RS485 adapter | Wallbox (Terra AC) |
|---|---|
| A (D+) | RS-485 A |
| B (D−) | RS-485 B |
| GND (optional) | GND (if available) |

**Configuration** (via Terra Config app → Communication Settings)

- Mode: **Modbus RTU**, charger as **secondary** device
- **Baud rate:** 57600, **Parity:** Even (8E1), **Stop bits:** 1
- **Modbus ID:** 9 (unique address, 1–247)

The adapter appears on the Pi as `/dev/ttyUSBEVSEcontrol` (udev symlink; the
bare device is usually `/dev/ttyUSB0`). Test with `mbpoll`:

```bash
sudo apt install mbpoll   # if not already installed

# Read charging state (register 400Ch)
mbpoll /dev/ttyUSBEVSEcontrol -m rtu -a 9 -c 2 -B -0 -1 -b 57600 -P even -s 1 -r 16396 -t 3:int

# Start charging session (register 4105h, value 0 = start)
mbpoll /dev/ttyUSBEVSEcontrol -m rtu -a 9 -B -0 -1 -b 57600 -P even -s 1 -r 16645 -t 3:int 0

# Set current limit to 16 A (register 4100h, value in mA)
mbpoll /dev/ttyUSBEVSEcontrol -m rtu -a 9 -B -0 -1 -b 57600 -P even -s 1 -r 16640 -t 3:int 16000
```

More registers and commands: `docs/wallbox/ABB_Terra_AC_Modbus_Befehle.md`
and the official datasheet `docs/wallbox/ABB_Terra_AC_Charger_ModbusCommunication_v1.7.pdf`.

**Critical notes**

- **Polling timeout:** the wallbox aborts the charging session if it is not
  polled within its communication timeout (**default 60 s**, register 4106h,
  settable 10–65535 s). Poll regularly (recommended 30–90 s) or raise the
  timeout.
- **Current limit < 6 A** puts the session into **Pause** (IEC 61851-1).
- Socket lock/unlock (register 4103h) only exists on socket models — the
  connector type is encoded in the serial number (register 4000h, byte 7).
- `1` = stop / lock, `0` = start / unlock (note the inverted logic on
  register 4105h!).

## 7. MQTT (next step)

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
