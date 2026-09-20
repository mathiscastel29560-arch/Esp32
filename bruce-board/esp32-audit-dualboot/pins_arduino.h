#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include "soc/soc_caps.h"
#include <stdint.h>

// Mirrors the audit-tool firmware's own include/config.h and HARDWARE.md
// exactly, so both firmwares on this dual-boot chip drive the same wires
// the same way. Values NOT re-derived here on purpose — copy config.h if
// either firmware's pinout ever changes.

// SERIAL (console UART, through the CP2102 bridge)
#define SERIAL_TX 43
#define SERIAL_RX 44
static const uint8_t TX = SERIAL_TX;
static const uint8_t RX = SERIAL_RX;
#define TX1 TX
#define RX1 RX

// I2C bus: DS3231 RTC (Bruce has no DS3231 driver, so this is unused on
// the Bruce side, but shares the same physical pins as our own firmware)
#define SYS_I2C_SDA 8
#define SYS_I2C_SCL 9
static const uint8_t SDA = SYS_I2C_SDA;
static const uint8_t SCL = SYS_I2C_SCL;

// UART1: NEO-6M GPS module
#define GPS_SERIAL_RX 18
#define GPS_SERIAL_TX 17

// Shared SPI bus: TFT + CC1101 + NRF24L01 (separate CS per device)
#define SPI_SCK_PIN 12
#define SPI_MOSI_PIN 11
#define SPI_MISO_PIN 13
#define SPI_SS_PIN 5 // TFT_CS; CC1101/NRF24 have their own CS below

static const uint8_t SS = SPI_SS_PIN;
static const uint8_t MOSI = SPI_MOSI_PIN;
static const uint8_t MISO = SPI_MISO_PIN;
static const uint8_t SCK = SPI_SCK_PIN;

// BUTTONS (4 physical buttons: no dedicated L/R, UP/DOWN double as
// Prev/Next in list navigation — see interface.cpp's InputHandler)
#define BTN_ALIAS "\"SEL\""
#define UP_BTN 1
#define DW_BTN 2
#define SEL_BTN 6
#define ESC_BTN 42
#define BTN_ACT LOW

// IR (Bruce's TXLED/RXLED naming is historical, not literal LEDs)
#define TXLED 38
#define RXLED 39
#define LED_ON HIGH
#define LED_OFF LOW

// CC1101 sub-GHz transceiver
#define USE_CC1101_VIA_SPI
#define CC1101_SS_PIN 10
#define CC1101_GDO0_PIN 4
#define CC1101_GDO2_PIN 40
#define CC1101_MOSI_PIN SPI_MOSI_PIN
#define CC1101_SCK_PIN SPI_SCK_PIN
#define CC1101_MISO_PIN SPI_MISO_PIN

// NRF24L01+PA/LNA 2.4GHz transceiver
#define USE_NRF24_VIA_SPI
#define NRF24_CE_PIN 15
#define NRF24_SS_PIN 14
#define NRF24_MOSI_PIN SPI_MOSI_PIN
#define NRF24_SCK_PIN SPI_SCK_PIN
#define NRF24_MISO_PIN SPI_MISO_PIN

// Battery voltage monitor (resistor-divider into ADC) — read generically
// by core/utils.cpp's getBattery() via ANALOG_BAT_PIN, see esp32-audit-dualboot.ini
#define ANALOG_BAT_PIN 7

// FONT SIZE: not overridden -- Bruce's own include/precompiler_flags.h
// already falls back to FP=1/FM=2/FG=3 via #ifndef when a board doesn't
// define them, and defining them here as plain integer macros pollutes
// every later use of the identifier "FP" -- including FastLED's own
// `using FP = fl::s16x16;` fixed-point type alias, which src/core/led_
// control.cpp pulls in unconditionally for every board.

// TFT_eSPI display: ILI9341 2.8" (same panel our own firmware drives)
#define HAS_SCREEN 1
#define ROTATION 1
#define MINBRIGHT (uint8_t)1

#define USER_SETUP_LOADED 1
#define ILI9341_DRIVER 1
#define TFT_WIDTH 240
#define TFT_HEIGHT 320
#define TFT_BACKLIGHT_ON HIGH
#define TFT_BL 48
#define TFT_RST -1 // tied to the board's own reset line
#define TFT_DC 16
#define TFT_MISO SPI_MISO_PIN
#define TFT_MOSI SPI_MOSI_PIN
#define TFT_SCLK SPI_SCK_PIN
#define TFT_CS 5
#define TOUCH_CS -1
#define SMOOTH_FONT 1
#define SPI_FREQUENCY 40000000
#define SPI_READ_FREQUENCY 20000000

// No SD card wired on this board
#define SDCARD_CS -1
#define SDCARD_SCK -1
#define SDCARD_MISO -1
#define SDCARD_MOSI -1

#endif /* Pins_Arduino_h */
