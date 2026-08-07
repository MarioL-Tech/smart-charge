#include <Arduino.h>
#include <ESP32Servo.h>
#include <SPI.h>
#include <MFRC522.h>

#include "config.h"

MFRC522 mfrc522(RFID_SS_PIN, RFID_RST_PIN);
Servo antiTheftServo;

// Charging is controlled by the Raspberry Pi (wallbox Modbus interface,
// see docs/uart-protocol.md). The ESP32 only mirrors the state.
bool chargingOn = false;

// Anti-theft is toggled independently by RFID taps (manual override):
// first tap unlocks, next tap locks. Starts ENGAGED (servo 90°) so the
// lock is closed at boot (fail-safe).
bool antiTheftActive = true;

unsigned long lastCardTapMs = 0;
bool lastCardPresent = false; // edge detection: a card must be removed before the next tap counts

// ---------- UART helpers ----------

String readUartLine() {
  static String line;
  while (Serial2.available()) {
    char c = (char)Serial2.read();
    if (c == '\n') {
      String out = line;
      line = "";
      return out;
    }
    if (c != '\r') line += c;
  }
  return "";
}

void sendStatus(const char *reason) {
  Serial2.printf("EVSE:STATUS:CHARGING:%s:SRC:%s\n",
                 chargingOn ? "ON" : "OFF", reason);
  Serial.printf("EVSE:STATUS:CHARGING:%s:SRC:%s\n",
                chargingOn ? "ON" : "OFF", reason);
}

void sendAntiTheft(const char *reason) {
  Serial2.printf("EVSE:STATUS:ANTITHEFT:%s:SRC:%s\n",
                 antiTheftActive ? "ACTIVE" : "INACTIVE", reason);
  Serial.printf("EVSE:STATUS:ANTITHEFT:%s:SRC:%s\n",
                antiTheftActive ? "ACTIVE" : "INACTIVE", reason);
}

void sendRfidEvent(const String &uidHex) {
  Serial2.printf("EVSE:RFID:CARD:UID:%s\n", uidHex.c_str());
  Serial.printf("EVSE:RFID:CARD:UID:%s\n", uidHex.c_str());
}

// ---------- Anti-theft servo ----------

// Sets the anti-theft lock state and reports every change to the Pi.
void setAntiTheft(bool state, const char *reason) {
  if (state == antiTheftActive) {
    return; // no change, do not spam the Pi
  }
  antiTheftActive = state;
  antiTheftServo.write(antiTheftActive ? SERVO_LOCK_DEG : SERVO_UNLOCK_DEG);
  sendAntiTheft(reason);
}

// RFID tap = manual override: first tap locks, next tap unlocks.
void toggleAntiTheft(const char *reason) {
  setAntiTheft(!antiTheftActive, reason);
}

// ---------- Charging state (Pi-controlled) ----------

void applyChargingState(bool state, const char *source) {
  if (state == chargingOn) {
    return; // no change, do not spam the Pi
  }
  chargingOn = state;
  sendStatus(source);
}

void handleUartCommand(const String &cmd) {
  if (cmd == "CMD:CHARGE:ON") {
    applyChargingState(true, "pi");
  } else if (cmd == "CMD:CHARGE:OFF") {
    applyChargingState(false, "pi");
  } else if (cmd == "CMD:STATUS") {
    sendStatus("query");
    sendAntiTheft("query");
  } else if (cmd.length() > 0) {
    // Unknown command: report back so the Pi side can notice protocol drift.
    Serial2.printf("EVSE:ERROR:UNKNOWN_CMD:%s\n", cmd.c_str());
  }
}

// ---------- RFID (manual override) ----------

String uidToHex() {
  String hex = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) hex += "0";
    hex += String(mfrc522.uid.uidByte[i], HEX);
    if (i < mfrc522.uid.size - 1) hex += ":";
  }
  hex.toUpperCase();
  return hex;
}

void handleRfidCard() {
  bool present = mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial();
  if (!present) {
    lastCardPresent = false; // card was removed, next detection is a new tap
    return;
  }

  unsigned long now = millis();
  if (lastCardPresent) {
    // Same card still on the reader: ignore re-reads while held.
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return;
  }
  lastCardPresent = true;

  // Debounce: minimum time between two accepted taps.
  if (now - lastCardTapMs < CARD_DEBOUNCE_MS) {
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return;
  }
  lastCardTapMs = now;

  String uid = uidToHex();
  sendRfidEvent(uid);

  // Manual override: any valid card toggles the anti-theft lock.
  // TODO: whitelist of allowed UIDs, see docs/uart-protocol.md.
  toggleAntiTheft("rfid");

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

// ---------- Main ----------

void setup() {
  Serial.begin(115200); // USB debug
  Serial2.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);

  SPI.begin(); // VSPI: SCK=18, MISO=19, MOSI=23
  mfrc522.PCD_Init();
  mfrc522.PCD_SetAntennaGain(MFRC522::RxGain_max);

  // Diagnostic: firmware version of the MFRC522.
  // 0x91/0x92 = module detected OK; 0x00/0xFF = wiring/power problem.
  uint8_t rfidVer = mfrc522.PCD_ReadRegister(MFRC522::VersionReg);
  Serial.printf("MFRC522 firmware version: 0x%02X\n", rfidVer);

  // Start with the anti-theft lock ENGAGED (fail-safe: locked at boot).
  antiTheftServo.attach(SERVO_PIN);
  antiTheftServo.write(SERVO_LOCK_DEG);

  Serial.println("EVSE RFID controller started");
  sendStatus("boot");
  sendAntiTheft("boot");
}

void loop() {
  handleRfidCard();

  String cmd = readUartLine();
  if (cmd.length() > 0) {
    handleUartCommand(cmd);
  }

  delay(10);
}
