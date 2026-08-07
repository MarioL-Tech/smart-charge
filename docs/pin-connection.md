# Pin Connection — Raspberry Pi ↔ ESP32

UART connection between the Raspberry Pi (40-pin header) and the ESP32 DevKit.
Both sides use **3.3V logic**, so no level shifter is required.

## Wiring

| Raspberry Pi (40-pin header) | ESP32 (DevKit) | Signal |
|---|---|---|
| GPIO 14 / TXD (pin 8) | GPIO 16 (Serial2 RX) | Pi TX → ESP32 RX |
| GPIO 15 / RXD (pin 10) | GPIO 17 (Serial2 TX) | Pi RX ← ESP32 TX |
| GND (pin 6) | GND | Common ground |

## Notes

- **Crossover:** TX of one side always goes to RX of the other side (never TX → TX).
- **Common ground (GND ↔ GND) is mandatory** — without it the UART signal has no reference and communication fails.
- **Never connect 5V to the ESP32 GPIOs** — the ESP32 is not 5V tolerant on most GPIOs.
- Serial2 on the ESP32 runs at 115200 baud (see `esp32/src/main.cpp`).
- The Raspberry Pi serial port must be enabled first: `sudo raspi-config` → Interface Options → Serial Port (login shell: No, hardware: Yes), then reboot. See `docs/setup.md`.

## MFRC522 RFID reader (manual override)

SPI connection between the MFRC522 module and the ESP32 DevKit.
VSPI defaults on the ESP32: SCK = GPIO18, MOSI = GPIO23, MISO = GPIO19.

| MFRC522 | ESP32 DevKit | Signal |
|---|---|---|
| SDA/SS | GPIO 5 | SPI chip select |
| SCK | GPIO 18 | SPI clock |
| MOSI | GPIO 23 | SPI MOSI |
| MISO | GPIO 19 | SPI MISO |
| RST | GPIO 22 | Reset |
| 3.3V | 3.3V | Power |
| GND | GND | Common ground |

Notes:

- The MFRC522 is a **3.3V device** — never connect it to 5V.
- The IRQ pin of the module is not connected (polling mode).
- A card tap acts as manual override and toggles the charging state (see `docs/uart-protocol.md`).
