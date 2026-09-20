#pragma once
// ============================================================================
// Hardware pin map — ESP32-S3-N16R8 audit tool
//
// This mirrors HARDWARE.md exactly (the locked pinout reference). GPIO
// 0,3,19,20,26-37,43-46 are avoided on purpose: strapping pins, native USB
// D+/D-, UART0 console (CP2102 bridge), or reserved for the Octal PSRAM /
// Quad flash on the N16R8 module.
//
// TFT pins are NOT redefined here: TFT_eSPI needs them as its own
// TFT_CS/TFT_DC/TFT_RST/TFT_BL macros, set once via platformio.ini
// build_flags (see that file) so the library picks them up at compile time.
// ============================================================================

// ---- I2C bus: DS3231 real-time clock, and (if wired instead of the TFT)
// the old SSD1306 OLED screen ----
#define PIN_I2C_SDA         8
#define PIN_I2C_SCL         9
#define RTC_I2C_ADDR        0x68

// ---- OLED screen (old, small): auto-detected at boot (see display.cpp)
// by probing this I2C address — if it doesn't answer, the firmware tries
// the TFT next. Most 1" SSD1306 modules are 128x64; some are 128x32 —
// change OLED_HEIGHT to match what you actually have.
#define OLED_I2C_ADDR       0x3C
#define OLED_WIDTH          128
#define OLED_HEIGHT         64

// ---- UART1: NEO-6M GPS module ----
#define PIN_GPS_RX          18   // ESP32 RX <- GPS TX
#define PIN_GPS_TX          17   // ESP32 TX -> GPS RX
#define GPS_BAUD            9600

// ---- Shared SPI bus: TFT + CC1101 + NRF24L01+PA/LNA (separate CS/aux per device) ----
#define PIN_SPI_SCK         12
#define PIN_SPI_MOSI        11
#define PIN_SPI_MISO        13

// CC1101 sub-GHz transceiver — 433MHz module (marked "433M" on the PCB)
#define PIN_CC1101_CS       10
#define PIN_CC1101_GDO0     4
#define PIN_CC1101_GDO2     40
#define CC1101_FREQ_MHZ     433.92f

// NRF24L01+PA/LNA 2.4GHz transceiver
#define PIN_NRF24_CS        14
#define PIN_NRF24_CE        15
#define PIN_NRF24_IRQ       41

// ---- Misc I/O ----
#define PIN_BUZZER          21

// IR receiver demodulator (VS1838B, 38kHz, 3-pin VCC/GND/OUT) and IR LED
// transmitter (940nm, through a transistor driver stage for range)
#define PIN_IR_TX           38
#define PIN_IR_RX           39

// ---- 4-button on-device menu (buttons to GND, INPUT_PULLUP) ----
#define PIN_BTN_UP          1
#define PIN_BTN_DOWN        2
#define PIN_BTN_SELECT      6
#define PIN_BTN_BACK        42

// ---- Battery voltage monitor (resistor-divider into ADC) ----
#define PIN_BATTERY_ADC     7

// NOTE: the slide switch is the device's power switch. It is wired in
// series with the battery, between TP4056 OUT+ and the MT3608 boost input
// (see HARDWARE.md §4) — NOT to a GPIO. There is nothing for the firmware
// to read: when it's off, the board has no power at all. TX-capable
// actions (deauth, beacon spam, evil portal, sub-GHz replay) are gated by
// physically holding the BACK button at the moment the action fires (see
// tx_arm.h) instead — GPIO47 is the only pin left free for a dedicated
// second switch if you'd rather have that back.

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

// ---- BLE audit suite thresholds (see FEATURES.md Module 3) ----
#define BLE_SPAM_DISTINCT_MAC_THRESHOLD  8   // distinct random MACs/sec advertising pairing beacons
#define BLE_SPAM_WINDOW_MS               1000
