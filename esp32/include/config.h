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
