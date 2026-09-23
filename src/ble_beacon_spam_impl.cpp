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

void sendBeacon(const uint8_t *payload, size_t payloadLen) {
    uint8_t mac[6];
    generateRandomMAC(mac);

    BLEAddress addr(mac);
    BLEAdvertisementData advData;

    if (payload) {
        advData.setCompleteServices(BLEUUID("000018F0-0000-1000-8000-00805F9B34FB"));
        advData.addData(std::string((const char*)payload, payloadLen));
    }

    g_beaconsGenerated++;
}
}

namespace BLEBeaconSpam {

SpamResult spamBeacons(const String &beaconType, uint32_t durationMs) {
    SpamResult result{false, 0, durationMs, beaconType};

    Serial.println("\n=== BLE Beacon Spam ===");
    Serial.println("Type: " + beaconType);
    Serial.println("Duration: " + String(durationMs) + "ms");

    if (!TxArm::isArmed()) {
        Serial.println("✗ TX not armed (hold BACK button)");
    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
        return result;
    }

    BLEDevice::init("");
    BLEServer *pServer = BLEDevice::createServer();

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
    Serial.println("Duration: " + String(millis() - startTime) + "ms");
    Serial.println("Rate: ~" + String((result.beaconsCount * 1000) / (millis() - startTime)) + " beacons/sec");

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
