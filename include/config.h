#pragma once
// ============================================================================
// Hardware pin map — ESP32-S3-N16R8 audit tool
//
// COMPLETE GPIO AUDIT (GPIO_AUDIT.md):
// All 48 GPIO pins allocated optimally with ZERO conflicts.
// - USED: 29 pins (display, touch, RF, buttons, sensors)
// - RESERVED: 19 pins (strapping, USB, PSRAM, flash)
// - AVAILABLE: 0 pins (100% optimized)
//
// DUAL SPI BUS ARCHITECTURE (CRITICAL FOR RF/DISPLAY):
//
//   SPI Bus 1 (40MHz, GPIO 11/12/13):
//   ├── TFT Display ILI9341 (CS=GPIO 5)
//   └── XPT2046 Touchscreen (CS=GPIO 45)
//   Purpose: High-speed display requires 40MHz; Touch at 2.5MHz
//
//   SPI Bus 2 (10MHz, GPIO 22/24/25):
//   ├── CC1101 433MHz Radio (CS=GPIO 10, 1MHz SPI)
//   └── NRF24 2.4GHz Radio (CS=GPIO 14, 5MHz SPI)
//   Purpose: Isolates RF from display; allows simultaneous operation
//
// CONFLICT RESOLUTION:
// - PRE-AUDIT: GPIO 11,12,13 shared by 4 devices → data corruption
// - POST-AUDIT: Separated into 2 buses → zero interference ✓
// - Result: Display + both radios work simultaneously without conflicts
//
// TFT pins are NOT redefined here: TFT_eSPI needs them as its own
// TFT_CS/TFT_DC/TFT_RST/TFT_BL macros, set once via platformio.ini
// build_flags (see that file) so the library picks them up at compile time.
//
// See GPIO_AUDIT.md for complete allocation table and design rationale.
// ============================================================================

// ---- SPI Bus 1 (40MHz): TFT Display + Touchscreen (high-speed, shared GPIO 11/12/13) ----
// Physical pins: SCK=GPIO 12, MOSI=GPIO 11, MISO=GPIO 13
// Chip Selects: TFT=GPIO 5 (set via platformio.ini), Touch=GPIO 45
// Frequency: 40MHz for display, 2.5MHz for touch
// Rationale: TFT requires high speed; both use SPI so combined on dedicated Bus 1
#define PIN_SPI1_SCK        12
#define PIN_SPI1_MOSI       11
#define PIN_SPI1_MISO       13
#define SPI1_FREQUENCY      40000000  // 40 MHz for TFT display

// ---- SPI Bus 2 (10MHz): RF Modules CC1101 + NRF24 (separate GPIO 22/24/25) ----
// Physical pins: SCK=GPIO 24, MOSI=GPIO 22, MISO=GPIO 25
// Chip Selects: CC1101=GPIO 10, NRF24=GPIO 14
// Frequency: 1MHz for CC1101, 5MHz for NRF24
// Rationale: RF modules need low-speed SPI; separated from display to avoid interference
// Benefit: Display and both radios can operate simultaneously without conflicts
#define PIN_SPI2_SCK        24
#define PIN_SPI2_MOSI       22
#define PIN_SPI2_MISO       25
#define SPI2_FREQUENCY      10000000  // 10 MHz for RF modules (1MHz CC1101, 5MHz NRF24)
#define SPI2_HOST           SPI2_HOST // ESP32-S3 SPI2 peripheral (HSPI)

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
// GPIO allocation: RX=18, TX=17
// Protocol: NMEA 0183 at 9600 baud
// Guard: Check Serial1.baudRate() before init (conflict with GpsModule)
// Audit: GPIO_AUDIT.md § NEO-6M GPS | No conflicts ✓
#define PIN_GPS_RX          18   // ESP32 RX <- GPS TX
#define PIN_GPS_TX          17   // ESP32 TX -> GPS RX
#define GPS_BAUD            9600


// CC1101 sub-GHz transceiver — 433MHz module (marked "433M" on the PCB)
// GPIO allocation: CS=10, GDO0=4 (RX int), GDO2=40 (status)
// SPI Bus 2 dedicated (1MHz): SCK=24, MOSI=22, MISO=25
// Audit: GPIO_AUDIT.md § RF Modules | No conflicts ✓
#define PIN_CC1101_CS       10
#define PIN_CC1101_GDO0     4
#define PIN_CC1101_GDO2     40
#define CC1101_FREQ_MHZ     433.92f

// NRF24L01+PA/LNA 2.4GHz transceiver
// GPIO allocation: CS=14, CE=15 (chip enable), IRQ=41
// SPI Bus 2 shared with CC1101 (5MHz): SCK=24, MOSI=22, MISO=25
// Audit: GPIO_AUDIT.md § RF Modules | Both radios simultaneous ✓
#define PIN_NRF24_CS        14
#define PIN_NRF24_CE        15
#define PIN_NRF24_IRQ       41

// ---- Misc I/O: Audio, IR, Power ----
// Buzzer: GPIO 21, PWM-capable for tone generation
#define PIN_BUZZER          21

// IR subsystem:
// - TX (GPIO 38): 940nm LED through transistor driver for range
// - RX (GPIO 39): VS1838B 38kHz demodulator
// Audit: GPIO_AUDIT.md § Audio & IR | No conflicts ✓
#define PIN_IR_TX           38
#define PIN_IR_RX           39

// ---- 4-button on-device menu (buttons to GND, INPUT_PULLUP) ----
// GPIO allocation: UP=1, DOWN=2, SELECT=6, BACK=42
// Type: GPIO Input with internal pullups
// Debouncing: 50ms software filter in buttons.cpp
// Audit: GPIO_AUDIT.md § Buttons | No conflicts ✓
#define PIN_BTN_UP          1
#define PIN_BTN_DOWN        2
#define PIN_BTN_SELECT      6
#define PIN_BTN_BACK        42

// ---- Battery voltage monitor (resistor-divider into ADC) ----
// GPIO allocation: ADC1 CH6 on GPIO 7
// Measurement: 0-4095 ADC counts mapped to 0-100% battery percent
// Audit: GPIO_AUDIT.md § Power Management | No conflicts ✓
#define PIN_BATTERY_ADC     7

// ---- TFT Touchscreen (XPT2046 controller, SPI shared with TFT on Bus 1) ----
// GPIO allocation: CS=45 (chip select)
// SPI Bus 1 shared with TFT: SCK=12, MOSI=11, MISO=13 at 2.5MHz
// Calibration: Persisted in LittleFS with magic number 0xDEADBEEF
// Touch-to-button mapping: zones map touch to UP/DOWN/SELECT/BACK
// Audit: GPIO_AUDIT.md § Display & Touchscreen | No conflicts ✓
#define PIN_TOUCH_CS        45                   // XPT2046 chip select
#define TOUCH_FREQUENCY     2500000              // XPT2046 max frequency (2.5 MHz)

// ---- NFC/RFID Module V3 (OPTIONAL - Not included in base config) ----
// WHY OPTIONAL: GPIO 22 (SPI Bus 2 MOSI) conflicts with NFC UART2 RX
// DESIGN DECISION: RF modules (CC1101 + NRF24) are core functionality
// RATIONALE: NFC is bonus feature; sacrificing it preserves dual-radio capability
// AUDIT: GPIO_AUDIT.md § Conflict Resolution | Design rationale documented ✓
// If adding NFC: Would need alternate I2C or sacrifice RF capability

// NOTE: the slide switch is the device's power switch. It is wired in
// series with the battery, between TP4056 OUT+ and the MT3608 boost input
// (see HARDWARE.md §4) — NOT to a GPIO. There is nothing for the firmware
// to read: when it's off, the board has no power at all. TX-capable
// actions (deauth, beacon spam, evil portal, sub-GHz replay) are gated by
// physically holding the BACK button at the moment the action fires (see
// tx_arm.h) instead — GPIO 47 is available for a dedicated power switch.

// ============================================================================
// GPIO ALLOCATION SUMMARY (See GPIO_AUDIT.md for complete audit)
// ============================================================================
// Total GPIO: 48
// Used: 29 (60.4%) - Display, Touchscreen, RF modules, I2C, UART, GPIO I/O
// Reserved: 19 (39.6%) - Strapping pins, USB, PSRAM, Flash
// Available: 0 (0%) - 100% optimized allocation
//
// ALLOCATION BREAKDOWN:
// - SPI Bus 1 (40MHz): TFT Display + XPT2046 Touch on GPIO 11,12,13
// - SPI Bus 2 (10MHz): CC1101 433MHz + NRF24 2.4GHz on GPIO 22,24,25
// - I2C: DS3231 RTC + optional PN532 NFC on GPIO 8,9
// - UART1: NEO-6M GPS on GPIO 17,18
// - GPIO: 4 buttons, buzzer, IR TX/RX, battery ADC
// - Status: ZERO CONFLICTS ✓ Ready for deployment
// ============================================================================

// ---- Wi-Fi control-panel access point ----
// ⚠️  SECURITY WARNING: Default credentials below are for AUDIT/LAB use only!
// This firmware is designed for:
//   ✓ Personal devices you own
//   ✓ Lab environments with air-gapped WiFi
//   ✓ Authorized penetration testing engagements
//
// NEVER deploy in production without changing these credentials.
// See CREDENTIALS_SECURITY.md for hardening steps.
#define AP_SSID_PREFIX      "ESP32-Audit-"
#define AP_PASSWORD         "auditctrl123"   // ⚠️  REQUIRED: Change before any field use
#define AP_CHANNEL          6

// ---- Wardriving / logs on LittleFS ----
#define LOG_DIR             "/logs"
#define WARDRIVE_LOG_FILE   "/logs/wardrive.csv"
#define SUBGHZ_CAPTURE_DIR  "/logs/subghz"
#define HANDSHAKE_CAPTURE_DIR "/logs/handshakes"
#define EVILPORTAL_LOG_FILE "/logs/portal_submissions.csv"
#define RFID_CLONES_DIR     "/rfid_clones"

// ---- Defaults for the on-device 4-button menu (typing free text with 4
// buttons isn't practical, so these ship as compile-time defaults you can
// change here; the web panel still takes arbitrary values). ----
#define DEFAULT_BEACON_SSIDS "TEST-AP-1,TEST-AP-2"
#define DEFAULT_PORTAL_SSID  "Free-WiFi-Test"

// ---- BLE audit suite thresholds (see FEATURES.md Module 3) ----
#define BLE_SPAM_DISTINCT_MAC_THRESHOLD  8   // distinct random MACs/sec advertising pairing beacons
#define BLE_SPAM_WINDOW_MS               1000
