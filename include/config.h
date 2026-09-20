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

// ---- Shared SPI bus: TFT + CC1101 + NRF24L01 (separate CS/aux per device) ----
#define PIN_SPI_SCK         12
#define PIN_SPI_MISO        13
#define PIN_SPI_MOSI        11

// ILI9341 2.8" TFT, 240x320 panel, driven in landscape (320x240)
#define PIN_TFT_CS          10
#define PIN_TFT_DC          14
#define PIN_TFT_RST         21
#define TFT_WIDTH           320
#define TFT_HEIGHT          240

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

// IR receiver demodulator (e.g. TSOP38238/VS1838B, 3-pin VCC/GND/OUT) and
// IR LED transmitter (through a transistor driver stage)
#define PIN_IR_RX           39   // freed up now that the slide switch is no longer a GPIO (see below)
#define PIN_IR_TX           42

// ---- Optional 4-button on-device menu (buttons to GND, INPUT_PULLUP) ----
#define PIN_BTN_UP          1
#define PIN_BTN_DOWN        2
#define PIN_BTN_SELECT      40
#define PIN_BTN_BACK        41

// NOTE: the slide switch is now the device's power switch. It is wired
// in series with the battery (between battery+ and the regulator/VBAT
// input), NOT to a GPIO — there is nothing for the firmware to read: when
// it's off, the board has no power at all. See README for the wiring
// diagram. TX-capable actions (deauth, beacon spam, evil portal, sub-GHz
// replay) are now gated by physically holding the BACK button at the
// moment the action fires (see tx_arm.h) instead.

// ---- Wi-Fi control-panel access point ----
#define AP_SSID_PREFIX      "ESP32-Audit-"
#define AP_PASSWORD         "auditctrl123"   // change before field use
#define AP_CHANNEL          6

// ---- Wardriving / logs on LittleFS ----
#define LOG_DIR             "/logs"
#define WARDRIVE_LOG_FILE   "/logs/wardrive.csv"
#define SUBGHZ_CAPTURE_DIR  "/logs/subghz"
#define EVILPORTAL_LOG_FILE "/logs/portal_submissions.csv"

// ---- Defaults for the on-device 4-button menu (typing free text with 4
// buttons isn't practical, so these ship as compile-time defaults you can
// change here; the web panel still takes arbitrary values). ----
#define DEFAULT_BEACON_SSIDS "TEST-AP-1,TEST-AP-2"
#define DEFAULT_PORTAL_SSID  "Free-WiFi-Test"
