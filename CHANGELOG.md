# Changelog

All notable changes to the EVSE-Control project. This file is **actively maintained** — every code, docs or infrastructure change is logged here with a timestamp. New entries are added at the top of the current day's section.

Format: `YYYY-MM-DD HH:MM` (Europe/Vienna)

## 2026-08-07

- **10:32** — **Feat: anti-theft servo** — model servo (GPIO 13) acts as anti-theft lock: 90° while charging is active, 0° when charging stops; new `EVSE:STATUS:ANTITHEFT:...` messages; ESP32Servo library added (PR #12).

## 2026-08-06

- **19:51** — **Feat: RFID manual override (MFRC522)** — ESP32 reads RFID cards as manual override (tap = toggle charging), reports status to the Pi and accepts Pi commands over UART; new line protocol in `docs/uart-protocol.md`; `rasppi/src/main.py` is now an interactive UART bridge (PR #9).
- **19:38** — Docs: rework `docs/pin-connection.md` (proper markdown table, signal column, GND ↔ GND) and sync wiring section in `docs/setup.md` (PR #7).
- **19:35** — Docs: Mario created `docs/pin-connection.md` (pin wiring reference).
- **19:33** — Docs: add `docs/setup.md` — full setup guide (Pi preparation, wiring, ESP32 flash, connection test, MQTT) (PR #6).
- **19:29** — Docs: initial `pin-connection` notes.
- **19:24** — Chore: document `pyserial` in `rasppi/requirements.txt` (PR #5).
- **18:29** — **Feat: UART connection test** (PR #4): rewrite `esp32/src/main.cpp` (heartbeat counter + echo via Serial2) and `rasppi/src/main.py` (serial listener + PING every 3 s).
- **17:40–17:51** — Chore: remove `desktop.ini`, add root `.gitignore` (PR #3).
- **~17:30** — **Milestone: UART connection between Pi and ESP32 verified in both directions** (`EVSE_TEST` lines + `ECHO:PING`). Fixes along the way: serial console disabled via raspi-config (login shell off), `python3-serial` installed.
- **14:26–16:00** — Infra: repo cloned to custos server; GitHub access via SSH key (account key) + fine-grained PAT (PR creation via API); branch+PR workflow established.
- **12:07–12:25** — Hardware setup: Raspberry Pi on WiFi ("Leeb"), SSH enabled, keyboard layout set to German (raspi-config).

## 2026-02-02

- **23:31** — Trial reading data ESP32 ↔ Raspi (initial UART attempt).
- **21:46** — Repo repaired, cleanup of initial commits.
