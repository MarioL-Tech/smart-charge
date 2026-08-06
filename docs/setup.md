# Setup Guide

How to set up the EVSE-Control project from scratch: Raspberry Pi, ESP32,
wiring, and the first UART connection test including the RFID manual override.

## 1. Requirements

**Hardware**

- Raspberry Pi (3 or 4) with Raspberry Pi OS (Bookworm) installed
- ESP32 DevKit (e.g. WROOM-32)
- MFRC522 RFID reader module (optional, manual override)
- 3 jumper wires (female–female) for the UART link
- 7 more jumper wires (female–female) for the MFRC522 (optional)
- microSD card for the Pi

**Software**

- Raspberry Pi OS with WiFi + SSH enabled
- `pyserial` on the Pi (Python serial library)
- PlatformIO + VS Code for flashing the ESP32
- Mosquitto (MQTT broker) — only needed for the planned MQTT phase (section 6)

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

(On Bookworm, bare `pip install` is blocked by PEP 668 — use apt or a venv.
`rasppi/requirements.txt` lists the versioned dependency.)

## 3. Wiring (ESP32 <-> Raspberry Pi)

Both sides use 3.3V logic — **no level shifter needed**. The detailed pin
reference lives in [docs/pin-connection.md](pin-connection.md).

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

The RFID reader acts as a manual override: tapping a card toggles charging
start/stop. See [docs/uart-protocol.md](uart-protocol.md) for the ESP32 ↔ Pi
protocol. The pins are defined in `esp32/include/config.h`.

## 4. Flash the ESP32

Open the `esp32/` folder in VS Code (PlatformIO extension installed), then:

```bash
pio run -t upload
```

The firmware implements the UART protocol from
[docs/uart-protocol.md](uart-protocol.md): it reports its charging state on
boot and on every state change, toggles charging when an RFID card is tapped
(manual override), and answers commands from the Pi. The MFRC522 library is
fetched automatically via `lib_deps` in `esp32/platformio.ini`.

Optional — watch the ESP32's own debug output (USB serial):

```bash
pio device monitor
```

You should see `EVSE RFID controller started` followed by
`EVSE:STATUS:CHARGING:OFF:SRC:boot`.

## 5. Run the UART bridge (connection test)

On the Pi, from the `rasppi/` directory:

```bash
python3 src/main.py
```

Expected output:

```text
EVSE UART bridge: listening on /dev/serial0 @ 115200 baud
Commands: 'on' | 'off' | 'status' | <raw line> | Ctrl+C to stop
RX <- ESP32: EVSE:STATUS:CHARGING:OFF:SRC:boot
```

The `EVSE:STATUS:...` line — sent by the ESP32 on boot — proves the
**ESP32 → Pi** direction works.

**Test Pi → ESP32:** type `status` and press Enter. The ESP32 replies with
`EVSE:STATUS:CHARGING:OFF:SRC:query`. `on` / `off` toggle the (internal)
charging state — expected reply `EVSE:STATUS:CHARGING:ON:SRC:pi` etc.
No physical load is switched yet; the charging-station interface is TBD
(see [docs/uart-protocol.md](uart-protocol.md)).

**Test the RFID manual override:** hold a card on the MFRC522 reader. You
should see

```text
RX <- ESP32: EVSE:RFID:CARD:UID:AB:CD:EF:12
RX <- ESP32: EVSE:STATUS:CHARGING:ON:SRC:rfid
```

Tapping again switches charging off.

**Troubleshooting**

- `raspberrypi login:` / `Password:` text in the output → the serial console
  is still enabled — repeat step 2 (Serial Port settings) and reboot.
- `PermissionError: [Errno 13]` on `/dev/serial0` → add your user to the
  `dialout` group: `sudo usermod -aG dialout $USER`, then log out and back in.
- No output at all → check the wiring: GND ↔ GND is mandatory, TX ↔ RX must
  be crossed (never TX ↔ TX).
- Garbage characters → wrong baud rate (must be 115200).

## 6. MQTT (planned)

Not implemented yet — the current ESP32 firmware is UART-only. In the MQTT
phase the UART messages will be published to a broker, most likely with the
Pi as the MQTT gateway (so the ESP32 does not need WiFi). Details are TBD.

Install the broker on the Pi:

```bash
sudo apt install mosquitto mosquitto-clients
```

Test locally:

```bash
mosquitto_sub -h localhost -t "evse/test"   # terminal 1
mosquitto_pub -h localhost -t "evse/test" -m "hello"   # terminal 2
```
