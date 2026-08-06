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
