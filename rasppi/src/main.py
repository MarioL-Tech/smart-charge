"""UART connection test: Raspberry Pi <-> ESP32.

Run this on the Pi while the ESP32 sketch (esp32/src/main.cpp) is running.
The ESP32 sends EVSE_TEST:<counter> every second and echoes PING as ECHO:PING.
Seeing lines on both RX and TX means the wiring is correct.

Note: enable the serial port first via `sudo raspi-config`
-> Interface Options -> Serial Port -> login shell: No, serial port: Yes.
"""

import serial
import time

PORT = "/dev/serial0"
BAUD = 115200

ser = serial.Serial(PORT, BAUD, timeout=1)
print(f"EVSE connection test: listening on {PORT} @ {BAUD} baud")
print("Press Ctrl+C to stop.")

last_ping = time.time()

try:
    while True:
        # Send a ping every 3 seconds to test the Pi -> ESP32 direction
        if time.time() - last_ping > 3:
            ser.write(b"PING\n")
            print("TX -> ESP32: PING")
            last_ping = time.time()

        line = ser.readline()
        if line:
            print(f"RX <- ESP32: {line.decode('utf-8', errors='replace').rstrip()}")
except KeyboardInterrupt:
    print("\nStopped by user")
finally:
    ser.close()
