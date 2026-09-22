#include "hardware.h"
#include "drivers/cc1101_driver.h"
#include "drivers/nrf24_driver.h"
#include "drivers/pn532_driver.h"
#include "drivers/gps_driver.h"
#include "drivers/rtc_driver.h"
#include "drivers/gpio_driver.h"

namespace Hardware {

static bool cc1101_ready = false;
static bool nrf24_ready = false;
static bool pn532_ready = false;
static bool gps_ready = false;
static bool rtc_ready = false;
static bool gpio_ready = false;

bool initAll() {
    Serial.println("\n╔════════════════════════════════════════════════════════════════╗");
    Serial.println("║            ESP32-S3 Hardware Initialization                     ║");
    Serial.println("╚════════════════════════════════════════════════════════════════╝\n");

    // GPIO first (buttons, buzzer, battery)
    Serial.println("[Hardware] 1/6 Initializing GPIO...");
    gpio_ready = GPIODriver::init();
    if (gpio_ready) {
        Serial.println("  ✓ Buttons, Buzzer, IR, Battery ready");
    } else {
        Serial.println("  ✗ GPIO initialization failed");
    }

    // RTC (time keeping)
    Serial.println("[Hardware] 2/6 Initializing RTC (I2C)...");
    rtc_ready = RTCDriver::init();
    if (rtc_ready) {
        RTCDriver::DateTime dt = RTCDriver::getDateTime();
        Serial.printf("  ✓ RTC ready - Time: %04d-%02d-%02d %02d:%02d:%02d\n",
                      dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
    } else {
        Serial.println("  ✗ RTC initialization failed (non-critical)");
    }

    // GPS (UART)
    Serial.println("[Hardware] 3/6 Initializing GPS (UART)...");
    gps_ready = GPSDriver::init();
    if (gps_ready) {
        Serial.println("  ✓ GPS ready - waiting for satellite fix...");
    } else {
        Serial.println("  ✗ GPS initialization failed");
    }

    // PN532 NFC (I2C)
    Serial.println("[Hardware] 4/6 Initializing PN532 NFC Reader (I2C)...");
    pn532_ready = PN532Driver::init();
    if (pn532_ready) {
        uint32_t fwVersion = PN532Driver::getFirmwareVersion();
        Serial.printf("  ✓ PN532 ready - Firmware version: 0x%08lX\n", fwVersion);
    } else {
        Serial.println("  ✗ PN532 initialization failed");
    }

    // CC1101 RF (SPI, 433 MHz)
    Serial.println("[Hardware] 5/6 Initializing CC1101 (433 MHz)...");
    CC1101Driver::Config cc1101_cfg = {
        .frequency = 433000000,
        .baudrate = 1000,
        .modulation = 0,  // FSK
        .rxEnabled = true,
        .txEnabled = true
    };
    cc1101_ready = CC1101Driver::init(cc1101_cfg);
    if (cc1101_ready) {
        Serial.println("  ✓ CC1101 ready");
    } else {
        Serial.println("  ✗ CC1101 initialization failed");
    }

    // NRF24 (SPI, 2.4 GHz)
    Serial.println("[Hardware] 6/6 Initializing NRF24 (2.4 GHz)...");
    NRF24Driver::Config nrf24_cfg = {
        .channel = 76,
        .payloadSize = 32,
        .addressWidth = 5,
        .rxEnabled = true,
        .txEnabled = true
    };
    nrf24_ready = NRF24Driver::init(nrf24_cfg);
    if (nrf24_ready) {
        Serial.println("  ✓ NRF24 ready");
    } else {
        Serial.println("  ✗ NRF24 initialization failed");
    }

    // Summary
    Serial.println("\n╔════════════════════════════════════════════════════════════════╗");
    Serial.printf("║ GPIO: %s | RTC: %s | GPS: %s | PN532: %s | CC1101: %s | NRF24: %s ║\n",
                  gpio_ready ? "✓" : "✗",
                  rtc_ready ? "✓" : "✗",
                  gps_ready ? "✓" : "✗",
                  pn532_ready ? "✓" : "✗",
                  cc1101_ready ? "✓" : "✗",
                  nrf24_ready ? "✓" : "✗");
    Serial.println("╚════════════════════════════════════════════════════════════════╝\n");

    bool anyReady = (cc1101_ready || nrf24_ready || pn532_ready || gps_ready || rtc_ready || gpio_ready);
    return anyReady;
}

bool isCC1101Ready() {
    return cc1101_ready;
}

bool isNRF24Ready() {
    return nrf24_ready;
}

bool isPN532Ready() {
    return pn532_ready;
}

bool isGPSReady() {
    return gps_ready;
}

bool isRTCReady() {
    return rtc_ready;
}

bool isBatteryReady() {
    return gpio_ready;
}

void shutdown() {
    Serial.println("[Hardware] Shutting down all modules...");

    if (cc1101_ready) CC1101Driver::deinit();
    if (nrf24_ready) NRF24Driver::deinit();
    if (pn532_ready) PN532Driver::deinit();
    if (gps_ready) GPSDriver::deinit();
    if (rtc_ready) RTCDriver::deinit();

    Serial.println("[Hardware] ✓ All modules shutdown");
}

String getDeviceName() {
    return "ESP32-S3 Offensive Security Platform v2.0";
}

}  // namespace Hardware
