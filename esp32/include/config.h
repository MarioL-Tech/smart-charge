#pragma once

// --- UART link ESP32 <-> Raspberry Pi ---
#define UART_BAUD     115200
#define UART_RX_PIN   16     // from Pi TXD (GPIO14)
#define UART_TX_PIN   17     // to Pi RXD (GPIO15)

// --- MFRC522 RFID reader (SPI) ---
// ESP32 DevKit VSPI defaults: SCK=GPIO18, MOSI=GPIO23, MISO=GPIO19
#define RFID_SS_PIN   5      // SDA/SS of the MFRC522
#define RFID_RST_PIN  22     // RST of the MFRC522

// --- Behavior ---
#define CARD_DEBOUNCE_MS 800 // ignore re-reads while the card is held on the reader

// --- Anti-theft servo ---
// Model servo (e.g. SG90) used as anti-theft lock. Toggled by RFID taps
// (manual override), independent of the charging state: first tap locks
// (SERVO_LOCK_DEG), next tap unlocks (SERVO_UNLOCK_DEG).
#define SERVO_PIN       13
#define SERVO_UNLOCK_DEG 0
#define SERVO_LOCK_DEG   90
