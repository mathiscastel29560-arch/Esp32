#pragma once
#include <Arduino.h>

// ============================================================
// HARDWARE TEST MODE - Initialize drivers in isolation
// ============================================================

namespace HardwareTestMode {

// Test flags - enable individual driver tests
struct TestConfig {
    bool test_gpio = false;
    bool test_rtc = false;
    bool test_gps = false;
    bool test_pn532 = false;
    bool test_cc1101 = false;
    bool test_nrf24 = false;
    bool verbose = true;
};

// Run isolated driver tests
void runTests(const TestConfig& config);

// Individual driver test functions
namespace Tests {
    void testGPIO();
    void testRTC();
    void testGPS();
    void testPN532();
    void testCC1101();
    void testNRF24();
}

}  // namespace HardwareTestMode
