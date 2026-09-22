#pragma once

// ============ GPIO PIN CONFIGURATION ============
// Centralized hardware pin definitions for ESP32-S3 DevKit

// ---- SPI Bus (for CC1101, NRF24L01, TFT) ----
#define SPI_CLK   18
#define SPI_MOSI  23
#define SPI_MISO  19

// ---- CC1101 Radio Module (433 MHz) ----
#define CC1101_CS    5
#define CC1101_GDO0  9    // Interrupt pin for RX
#define CC1101_GDO2  10   // Optional second interrupt

// ---- NRF24L01 Radio Module (2.4 GHz) ----
#define NRF24_CS   8
#define NRF24_CE   12

// ---- PN532 NFC/RFID Module ----
// I2C Address: 0x24
#define PN532_I2C_SDA  20
#define PN532_I2C_SCL  21

// ---- GPS NEO-6M Module ----
// UART2 (GPS uses Serial2 on ESP32)
#define GPS_RX  16
#define GPS_TX  17
#define GPS_BAUD 9600

// ---- RTC DS3231 Module ----
// I2C Address: 0x68 (shares I2C with PN532)
#define RTC_I2C_SDA  20
#define RTC_I2C_SCL  21

// ---- IR Receiver/Transmitter (GPIO) ----
#define IR_RX_PIN  4    // Receiver (input)
#define IR_TX_PIN  11   // Transmitter (output)

// ---- Buzzer (GPIO PWM) ----
#define BUZZER_PIN  13
#define BUZZER_CHANNEL 0
#define BUZZER_FREQ 1000

// ---- Button Controls (GPIO Input) ----
#define BTN_UP    14    // Navigation up
#define BTN_DOWN  15    // Navigation down
#define BTN_OK    6     // Select/OK
#define BTN_BACK  7     // Back/Return

// ---- Slide Switch (Power/Mode) ----
#define SWITCH_POWER  2

// ---- Battery Monitoring (ADC) ----
#define BATTERY_ADC_PIN  3   // GPIO3 = ADC1_CH2
#define BATTERY_ADC_CH   ADC1_CHANNEL_2
#define BATTERY_VOLTAGE_DIVIDER 2.0  // Voltage divider ratio (measure formula)

// ---- TFT Display (SPI) ----
#define TFT_CS   46
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

