#include "subghz_jammer_suite.h"
#include "tx_arm.h"
#include "config.h"
#include <RadioLib.h>

namespace {
Module cc1101Module(PIN_CC1101_CS, PIN_CC1101_GDO0, RADIOLIB_NC, PIN_CC1101_GDO2, SPI);
CC1101 radio(&cc1101Module);
volatile bool g_jamActive = false;
uint32_t g_jamCount = 0;

void sendResetCode(uint32_t code) {
    radio.begin(433.92f);
    radio.setOOK(true);
    radio.transmitDirectAsync();

    for (int i = 31; i >= 0; i--) {
        bool bit = (code >> i) & 1;
        digitalWrite(PIN_CC1101_GDO0, bit);
        delayMicroseconds(500);
    }
    g_jamCount++;
}
}

namespace SubghzJammerSuite {

JamResult jamSubghzDevices(uint32_t durationMs) {
    JamResult result{false, 0, durationMs};

    Serial.println("\n=== Sub-GHz Jammer Suite ===");
    Serial.println("Frequency: 433.92 MHz");
    Serial.println("Duration: " + String(durationMs) + "ms");

    if (!TxArm::isArmed()) {
        Serial.println("✗ TX not armed");
        return result;
    }

    g_jamActive = true;
    g_jamCount = 0;
    uint32_t startTime = millis();

    while (millis() - startTime < durationMs && g_jamActive) {
        // Send random reset/jam codes
        uint32_t jamCode = (esp_random() % 0xFFFFFFFF);
        sendResetCode(jamCode);

        delay(50);

        if (g_jamCount % 20 == 0) {
            Serial.println("  [" + String(g_jamCount) + "] jam codes");
        }
    }

    g_jamActive = false;
    result.success = true;
    result.jamPacketsCount = g_jamCount;

    Serial.println("✓ Complete: " + String(g_jamCount) + " jam codes");
    return result;
}

void stop() {
    g_jamActive = false;
}

bool isActive() {
    return g_jamActive;
}

}  // namespace SubghzJammerSuite
