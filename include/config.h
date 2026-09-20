#pragma once

// ---- GPIO Pinout (ESP32-S3-N16R8) ----
#define PIN_I2C_SDA           8
#define PIN_I2C_SCL           9
#define PIN_SPI_CLK           12
#define PIN_SPI_MOSI          11
#define PIN_SPI_MISO          13
#define PIN_CC1101_CSN        10
#define PIN_CC1101_GDO0       4
#define PIN_CC1101_GDO2       40
#define PIN_NRF24_CSN         14
#define PIN_NRF24_CE          15
#define PIN_NRF24_IRQ         41
#define PIN_TFT_CS            5
#define PIN_TFT_DC            16
#define PIN_TFT_BLK           48
#define PIN_GPS_RX            18
#define PIN_GPS_TX            17
#define PIN_BUZZER            21
#define PIN_BTN_UP            1
#define PIN_BTN_DOWN          2
#define PIN_BTN_OK            6
#define PIN_BTN_BACK          42
#define PIN_BATTERY_ADC       7
#define PIN_IR_TX             38
#define PIN_IR_RX             39

// ---- Wardriving / logs on LittleFS ----
#define LOG_DIR              "/logs"
#define WARDRIVE_LOG_FILE    "/logs/wardrive.csv"
#define SUBGHZ_CAPTURE_DIR   "/logs/subghz"
#define EVILPORTAL_LOG_FILE  "/logs/portal_submissions.csv"
#define HANDSHAKE_CAPTURE_DIR "/logs/handshakes"
#define RFID_CLONES_DIR      "/rfid_clones"

// ---- GPS Configuration ----
#define GPS_BAUD                        9600

// ---- OLED Display Configuration (SSD1306) ----
#define OLED_WIDTH                      128
#define OLED_HEIGHT                     64
#define OLED_I2C_ADDR                   0x3C

// ---- BLE Spam Detector Configuration ----
#define BLE_SPAM_WINDOW_MS              5000
#define BLE_SPAM_DISTINCT_MAC_THRESHOLD 10

// ---- Fun Features Toggle ----
// NOTE: These are now runtime configurable via Settings menu
// Default values are set in settings.cpp (all true/1 by default)
// Changes persist in LittleFS at /config/settings.json
// See: include/settings.h and src/settings.cpp
// Usage: Check Settings::g_config.audioEffects, etc. in runtime code
