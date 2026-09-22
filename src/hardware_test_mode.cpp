#include "hardware_test_mode.h"
#include "debug_logger.h"
#include "drivers/gpio_driver.h"
#include "drivers/rtc_driver.h"
#include "drivers/gps_driver.h"
#include "drivers/pn532_driver.h"
#include "drivers/cc1101_driver.h"
#include "drivers/nrf24_driver.h"
#include "hw_config.h"

namespace HardwareTestMode {

namespace Tests {

void testGPIO() {
    DBG_INFO("═══════════════════════════════════════════");
    DBG_INFO("GPIO ISOLATED TEST");
    DBG_INFO("═══════════════════════════════════════════");

    if (!GPIODriver::init()) {
        DBG_ERROR("GPIO initialization failed!");
        return;
    }

    DBG_INFO("✓ GPIO initialized");

    // Test buttons
    DBG_INFO("Testing buttons (press each one)...");
    for (int i = 0; i < 50; i++) {  // 5 seconds at 100ms polling
        uint8_t pins[] = {BTN_UP, BTN_DOWN, BTN_OK, BTN_BACK};
        const char* names[] = {"UP", "DOWN", "OK", "BACK"};

        for (int j = 0; j < 4; j++) {
            GPIODriver::ButtonEvent evt = GPIODriver::getButtonState(pins[j]);
            if (evt != GPIODriver::BUTTON_NONE) {
                DBG_INFO("  Button %s: Event %d", names[j], evt);
            }
        }
        delay(100);
    }

    // Test buzzer
    DBG_INFO("Testing buzzer (3 beeps)...");
    for (int i = 0; i < 3; i++) {
        GPIODriver::buzzerBeep(200);
        delay(300);
    }
    DBG_INFO("✓ Buzzer test complete");

    // Test battery
    DBG_INFO("Reading battery voltage...");
    uint8_t percent = GPIODriver::getBatteryPercent();
    float voltage = GPIODriver::getBatteryVoltage();
    DBG_INFO("  Battery: %d%% (%.2f V)", percent, voltage);

    DBG_INFO("═══════════════════════════════════════════");
    DBG_INFO("✓ GPIO TEST PASSED");
    DBG_INFO("═══════════════════════════════════════════\n");
}

void testRTC() {
    DBG_INFO("═══════════════════════════════════════════");
    DBG_INFO("RTC (DS3231) ISOLATED TEST");
    DBG_INFO("═══════════════════════════════════════════");

    if (!RTCDriver::init()) {
        DBG_ERROR("RTC initialization failed!");
        DBG_ERROR("Possible causes:");
        DBG_ERROR("  - DS3231 module not connected");
        DBG_ERROR("  - I2C bus error (SDA=%d, SCL=%d)", RTC_I2C_SDA, RTC_I2C_SCL);
        DBG_ERROR("  - Address mismatch (expected 0x%02X)", RTC_I2C_ADDR);
        return;
    }

    DBG_INFO("✓ RTC initialized");

    // Read time
    RTCDriver::DateTime dt = RTCDriver::getDateTime();
    DBG_INFO("Current time: %04d-%02d-%02d %02d:%02d:%02d",
             dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);

    // Read temperature
    float temp = RTCDriver::getTemperature();
    DBG_INFO("Temperature: %.2f°C", temp);

    // Check battery
    bool battery_low = RTCDriver::isBatteryLow();
    DBG_INFO("Battery status: %s", battery_low ? "LOW" : "OK");

    DBG_INFO("═══════════════════════════════════════════");
    DBG_INFO("✓ RTC TEST PASSED");
    DBG_INFO("═══════════════════════════════════════════\n");
}

void testGPS() {
    DBG_INFO("═══════════════════════════════════════════");
    DBG_INFO("GPS (NEO-6M) ISOLATED TEST");
    DBG_INFO("═══════════════════════════════════════════");

    // Check if GpsModule already active
    if (Serial1.baudRate() > 0) {
        DBG_WARN("UART1 already in use (GpsModule active)");
        DBG_WARN("Skipping GPSDriver test - use GpsModule instead");
        DBG_INFO("═══════════════════════════════════════════\n");
        return;
    }

    if (!GPSDriver::init()) {
        DBG_ERROR("GPS initialization failed!");
        DBG_ERROR("Possible causes:");
        DBG_ERROR("  - GPS module not connected");
        DBG_ERROR("  - UART1 error (RX=%d, TX=%d, Baud=%d)", GPS_RX, GPS_TX, GPS_BAUD);
        return;
    }

    DBG_INFO("✓ GPS initialized");
    DBG_INFO("Waiting for satellite fix (up to 30 seconds)...");

    uint32_t start = millis();
    uint32_t last_update = start;

    while (millis() - start < 30000) {
        GPSDriver::update();

        if (millis() - last_update > 2000) {  // Update every 2 seconds
            if (GPSDriver::hasFix()) {
                GPSDriver::Location loc = GPSDriver::getLocation();
                DBG_INFO("  ✓ GPS Fix! Lat: %.6f, Lon: %.6f, Alt: %.1f m, Sats: %d",
                         loc.latitude, loc.longitude, loc.altitude, loc.satellites);
                break;
            } else {
                DBG_VERBOSE("  Waiting for fix... (%d/%d sec)",
                           (millis() - start) / 1000, 30);
            }
            last_update = millis();
        }
        delay(100);
    }

    DBG_INFO("═══════════════════════════════════════════");
    DBG_INFO("✓ GPS TEST COMPLETE");
    DBG_INFO("═══════════════════════════════════════════\n");
}

void testPN532() {
    DBG_INFO("═══════════════════════════════════════════");
    DBG_INFO("PN532 NFC ISOLATED TEST");
    DBG_INFO("═══════════════════════════════════════════");

    if (!PN532Driver::init()) {
        DBG_ERROR("PN532 initialization failed!");
        DBG_ERROR("Possible causes:");
        DBG_ERROR("  - PN532 module not connected");
        DBG_ERROR("  - I2C bus error (SDA=%d, SCL=%d)", PN532_I2C_SDA, PN532_I2C_SCL);
        DBG_ERROR("  - Wrong I2C address (expected 0x%02X)", PN532_I2C_ADDR);
        return;
    }

    DBG_INFO("✓ PN532 initialized");

    uint32_t fw = PN532Driver::getFirmwareVersion();
    DBG_INFO("Firmware version: 0x%08lX", fw);

    DBG_INFO("Scanning for NFC cards (10 seconds)...");
    uint32_t start = millis();
    bool found = false;

    while (millis() - start < 10000) {
        PN532Driver::Card card = PN532Driver::scanCard();

        if (card.hasCard) {
            DBG_INFO("  ✓ Card detected!");
            DBG_INFO("    UID: %s", PN532Driver::getUIDString(card.uid, card.uidLength).c_str());
            DBG_INFO("    Type: %s", card.cardType);
            found = true;
            break;
        }
        delay(500);
    }

    if (!found) {
        DBG_WARN("No cards detected - place card near reader");
    }

    DBG_INFO("═══════════════════════════════════════════");
    DBG_INFO("✓ PN532 TEST COMPLETE");
    DBG_INFO("═══════════════════════════════════════════\n");
}

void testCC1101() {
    DBG_INFO("═══════════════════════════════════════════");
    DBG_INFO("CC1101 (433MHz) ISOLATED TEST");
    DBG_INFO("═══════════════════════════════════════════");

    DBG_WARN("Testing CC1101 - Ensure SubGhz module is DISABLED");

    CC1101Driver::Config cfg = {
        .frequency = 433000000,
        .baudrate = 1000,
        .modulation = 0,  // FSK
        .rxEnabled = true,
        .txEnabled = true
    };

    if (!CC1101Driver::init(cfg)) {
        DBG_ERROR("CC1101 initialization failed!");
        DBG_ERROR("Possible causes:");
        DBG_ERROR("  - CC1101 module not connected");
        DBG_ERROR("  - SPI bus error (SCK=%d, MOSI=%d, MISO=%d)", SPI_CLK, SPI_MOSI, SPI_MISO);
        DBG_ERROR("  - Wrong CS pin (GPIO%d)", CC1101_CS);
        DBG_ERROR("  - SubGhz module active (conflicts with CC1101Driver)");
        return;
    }

    DBG_INFO("✓ CC1101 initialized");
    DBG_INFO("Listening for 433MHz signals (10 seconds)...");

    uint32_t start = millis();
    while (millis() - start < 10000) {
        int rssi = CC1101Driver::getRSSI();
        DBG_VERBOSE("  RSSI: %d dBm", rssi);

        if (CC1101Driver::isRXReady()) {
            DBG_INFO("  ✓ Signal detected! RSSI: %d dBm", rssi);
        }

        delay(500);
    }

    DBG_INFO("═══════════════════════════════════════════");
    DBG_INFO("✓ CC1101 TEST COMPLETE");
    DBG_INFO("═══════════════════════════════════════════\n");
}

void testNRF24() {
    DBG_INFO("═══════════════════════════════════════════");
    DBG_INFO("NRF24 (2.4GHz) ISOLATED TEST");
    DBG_INFO("═══════════════════════════════════════════");

    DBG_INFO("Testing NRF24 - SPI bus shared with TFT");

    NRF24Driver::Config cfg = {
        .channel = 76,
        .payloadSize = 32,
        .addressWidth = 5,
        .rxEnabled = true,
        .txEnabled = true
    };

    if (!NRF24Driver::init(cfg)) {
        DBG_ERROR("NRF24 initialization failed!");
        DBG_ERROR("Possible causes:");
        DBG_ERROR("  - NRF24 module not connected");
        DBG_ERROR("  - SPI bus error (SCK=%d, MOSI=%d, MISO=%d)", SPI_CLK, SPI_MOSI, SPI_MISO);
        DBG_ERROR("  - Wrong pins (CS=%d, CE=%d)", NRF24_CS, NRF24_CE);
        DBG_ERROR("  - Display/TFT initialization interfering");
        return;
    }

    DBG_INFO("✓ NRF24 initialized");
    DBG_INFO("Scanning 2.4GHz channels (20 seconds)...");

    // Test channel hopping
    for (int ch = 0; ch < 5; ch++) {
        NRF24Driver::setChannel(ch * 20);
        int rssi = NRF24Driver::getRSSI();
        DBG_INFO("  Channel %d (%.1f MHz): RSSI %d dBm",
                 ch * 20, 2400.0 + ch * 20, rssi);
        delay(4000);
    }

    DBG_INFO("═══════════════════════════════════════════");
    DBG_INFO("✓ NRF24 TEST COMPLETE");
    DBG_INFO("═══════════════════════════════════════════\n");
}

}  // namespace Tests

void runTests(const TestConfig& config) {
    DBG_INFO("\n");
    DBG_INFO("╔════════════════════════════════════════════════════════════╗");
    DBG_INFO("║        HARDWARE TEST MODE - ISOLATED DRIVER TESTS          ║");
    DBG_INFO("╚════════════════════════════════════════════════════════════╝");

    DBG_MEMORY("Test Start");

    if (config.test_gpio)   Tests::testGPIO();
    if (config.test_rtc)    Tests::testRTC();
    if (config.test_gps)    Tests::testGPS();
    if (config.test_pn532)  Tests::testPN532();
    if (config.test_cc1101) Tests::testCC1101();
    if (config.test_nrf24)  Tests::testNRF24();

    DBG_MEMORY("Test Complete");

    DBG_INFO("╔════════════════════════════════════════════════════════════╗");
    DBG_INFO("║                 ALL TESTS COMPLETED                        ║");
    DBG_INFO("╚════════════════════════════════════════════════════════════╝\n");
}

}  // namespace HardwareTestMode
