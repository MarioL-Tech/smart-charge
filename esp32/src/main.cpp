#include <Arduino.h>

// UART connection test: ESP32 <-> Raspberry Pi
// Serial2: GPIO 16 = RX (from Pi TXD/GPIO14), GPIO 17 = TX (to Pi RXD/GPIO15)
// Run rasppi/src/main.py on the Pi side to see the data.

void setup() {
  Serial.begin(115200);                        // USB serial (debug)
  Serial2.begin(115200, SERIAL_8N1, 16, 17);   // UART to the Pi
}

void loop() {
  static uint32_t counter = 0;

  // Heartbeat: send a counter line to the Pi every second
  Serial2.printf("EVSE_TEST:%lu\n", counter);
  Serial.printf("TX -> Pi: EVSE_TEST:%lu\n", counter);
  counter++;

  // Echo anything the Pi sends back (proves RX direction)
  if (Serial2.available()) {
    String line = Serial2.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      Serial2.printf("ECHO:%s\n", line.c_str());
      Serial.printf("RX <- Pi: %s\n", line.c_str());
    }
  }

  delay(1000);
}
