#pragma once

// ---- SPI (shared by TFT + RFID) ----
#define SPI_CLK              12   // SCLK
#define SPI_MOSI             11   // MOSI / SDI
#define SPI_MISO             13   // MISO / SDO

// ---- TFT Display (SPI-based, e.g., ST7735 / ILI9341) ----
#define TFT_CS               10   // Chip Select
#define TFT_DC               9    // Data/Command
#define TFT_RST              6    // Reset
#define TFT_WIDTH            128  // or 240 depending on model
#define TFT_HEIGHT           64   // or 320 depending on model

// ---- RFID (RC522 via SPI) ----
#define RFID_CS              8    // Chip Select for RC522
#define RFID_RST             7    // Reset for RC522
#define RFID_FREQ            13.56 // MHz (ISO14443A standard)

// ---- Wardriving / logs on LittleFS ----
#define LOG_DIR              "/logs"
#define WARDRIVE_LOG_FILE    "/logs/wardrive.csv"
#define SUBGHZ_CAPTURE_DIR   "/logs/subghz"
#define EVILPORTAL_LOG_FILE  "/logs/portal_submissions.csv"
#define HANDSHAKE_CAPTURE_DIR "/logs/handshakes"
#define RFID_CLONES_DIR      "/logs/rfid"
