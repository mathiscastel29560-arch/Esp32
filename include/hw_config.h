#pragma once
#include "config.h"

// ============ HARDWARE DRIVER PIN CONFIGURATION ============
// Maps from config.h to driver-specific constants
// All pins are locked to project HARDWARE.md pinout

// ---- SPI Bus (Shared by TFT, CC1101, NRF24L01) ----
#define SPI_CLK    PIN_SPI_SCK     // 12
#define SPI_MOSI   PIN_SPI_MOSI    // 11
#define SPI_MISO   PIN_SPI_MISO    // 13

// ---- CC1101 Radio Module (433 MHz) ----
#define CC1101_CS    PIN_CC1101_CS      // 10
#define CC1101_GDO0  PIN_CC1101_GDO0    // 4 (interrupt pin for RX)
#define CC1101_GDO2  PIN_CC1101_GDO2    // 40
#define CC1101_FREQ  CC1101_FREQ_MHZ    // 433.92 MHz

// ---- NRF24L01+PA/LNA Radio Module (2.4 GHz) ----
#define NRF24_CS   PIN_NRF24_CS    // 14
#define NRF24_CE   PIN_NRF24_CE    // 15
#define NRF24_IRQ  PIN_NRF24_IRQ   // 41

// ---- PN532 NFC/RFID Module (I2C) ----
// Uses shared I2C bus with RTC
#define PN532_I2C_SDA   PIN_I2C_SDA    // 8
#define PN532_I2C_SCL   PIN_I2C_SCL    // 9
#define PN532_I2C_ADDR  0x24           // PN532 I2C address

// ---- GPS NEO-6M Module (UART1) ----
#define GPS_RX     PIN_GPS_RX      // 18 (ESP32 RX <- GPS TX)
#define GPS_TX     PIN_GPS_TX      // 17 (ESP32 TX -> GPS RX)
#define GPS_UART   1               // UART1 (Serial1)
#define GPS_BAUD   9600            // Baud rate

// ---- RTC DS3231 Module (I2C) ----
// Shares I2C bus 0 with PN532 (SDA=8, SCL=9)
#define RTC_I2C_SDA   PIN_I2C_SDA    // 8
#define RTC_I2C_SCL   PIN_I2C_SCL    // 9
#define RTC_I2C_ADDR  0x68           // I2C address

// ---- IR Receiver/Transmitter (GPIO) ----
#define IR_RX_PIN  PIN_IR_RX    // 39 (receiver input)
#define IR_TX_PIN  PIN_IR_TX    // 38 (transmitter output)

// ---- Buzzer (GPIO PWM) ----
#define BUZZER_PIN      PIN_BUZZER     // 21
#define BUZZER_CHANNEL  0              // PWM channel 0
#define BUZZER_FREQ     1000           // Base frequency 1 kHz

// ---- Button Controls (GPIO Input) ----
#define BTN_UP     PIN_BTN_UP        // 1
#define BTN_DOWN   PIN_BTN_DOWN      // 2
#define BTN_OK     PIN_BTN_SELECT    // 6
#define BTN_BACK   PIN_BTN_BACK      // 42

// ---- Battery Monitoring (ADC) ----
#define BATTERY_ADC_PIN  PIN_BATTERY_ADC    // 7
#define BATTERY_VOLTAGE_DIVIDER 2.0         // 2:1 voltage divider
#define TFT_DC   47
#define TFT_RST  48
#define TFT_BL   -1    // No backlight PWM (always on)

// ---- OLED Display (I2C) ----
// I2C Address: 0x3C
#define OLED_I2C_SDA  20
#define OLED_I2C_SCL  21

// ============ CONFIGURATION CONSTANTS ============

// CC1101 Configuration
#define CC1101_FREQ_433 433000000  // 433 MHz
#define CC1101_BAUDRATE 57600

// NRF24 Configuration
#define NRF24_FREQ_2400 2400       // 2.4 GHz (MHz)
#define NRF24_CHANNEL 76           // Default WiFi interference-free channel
#define NRF24_POWER PA_MAX
#define NRF24_SPEED RF24_250KBPS

// GPS Configuration
#define GPS_TIMEOUT_MS 5000        // Timeout waiting for GPS fix

// RTC Configuration
#define RTC_I2C_SPEED 100000       // 100 kHz I2C speed for RTC

// Battery Configuration
#define BATTERY_SAMPLES 10         // Average over N samples
#define BATTERY_LOW_THRESHOLD 15   // % - low battery warning
#define BATTERY_CRITICAL_THRESHOLD 5 // % - shutdown threshold

// Button Configuration
#define BUTTON_DEBOUNCE_MS 20      // Debounce delay
#define BUTTON_LONG_PRESS_MS 1000  // Time for long press detection

// IR Configuration
#define IR_CARRIER_FREQ 38000      // Standard IR carrier frequency (Hz)
#define IR_REC_BUFFER_SIZE 256     // IR receiver buffer

