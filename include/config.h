#pragma once
// ============================================================================
// Hardware pin map — ESP32-S3-N16R8 audit tool
//
// GPIO0,3,19,20,26-37,43-46 are avoided on purpose: strapping pins, native
// USB D+/D-, UART0 console, or reserved for the Octal PSRAM/Quad flash on
// the N16R8 module. Every pin below is free for general use on that module.
// ============================================================================

// ---- I2C bus: DS3231 real-time clock ----
#define PIN_I2C_SDA         8
#define PIN_I2C_SCL         9
#define RTC_I2C_ADDR        0x68

// ---- UART1: NEO-6M GPS module ----
#define PIN_GPS_RX          17   // ESP32 RX <- GPS TX
#define PIN_GPS_TX          18   // ESP32 TX -> GPS RX
#define GPS_BAUD            9600

// ---- Shared SPI bus: OLED + CC1101 + NRF24L01 (separate CS/aux per device) ----
#define PIN_SPI_SCK         12
#define PIN_SPI_MISO        13
#define PIN_SPI_MOSI        11

// SSD1306 OLED (SPI mode)
#define PIN_OLED_CS         10
#define PIN_OLED_DC         14
#define PIN_OLED_RST        21
#define OLED_WIDTH          128
#define OLED_HEIGHT         64   // change to 32 if your 1" panel is 128x32

// CC1101 sub-GHz transceiver
#define PIN_CC1101_CS       15
#define PIN_CC1101_GDO0     16
#define PIN_CC1101_GDO2     4

// NRF24L01 2.4GHz transceiver
#define PIN_NRF24_CS        5
#define PIN_NRF24_CE        6
#define PIN_NRF24_IRQ       7

// ---- Misc I/O ----
#define PIN_BUZZER          38
#define PIN_SAFETY_SWITCH   39   // slide switch: HIGH = TX/injection features armed

// ---- Wi-Fi control-panel access point ----
#define AP_SSID_PREFIX      "ESP32-Audit-"
#define AP_PASSWORD         "auditctrl123"   // change before field use
#define AP_CHANNEL          6

// ---- Wardriving / logs on LittleFS ----
#define LOG_DIR             "/logs"
#define WARDRIVE_LOG_FILE   "/logs/wardrive.csv"
#define SUBGHZ_CAPTURE_DIR  "/logs/subghz"
