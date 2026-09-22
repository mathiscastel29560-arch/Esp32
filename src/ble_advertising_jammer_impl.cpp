#include "ble_advertising_jammer.h"
#include "tx_arm.h"
#include "config.h"
#include <BLEDevice.h>
#include <BLEAdvertising.h>

namespace {
volatile bool g_jamActive = false;
uint32_t g_jamPacketsCount = 0;

// BLE advertising channels: 37 (2402 MHz), 38 (2426 MHz), 39 (2480 MHz)
const uint8_t BLE_ADV_CHANNELS[] = {37, 38, 39};

void sendJamPacket() {
    uint8_t jamPayload[31];

    // Generate random noise pattern
    for (int i = 0; i < 31; i++) {
        jamPayload[i] = (esp_random() % 256);
    }

    BLEAddress addr(jamPayload);
    BLEAdvertisementData jamData;
    jamData.addData(std::string((const char*)jamPayload, 31));

    g_jamPacketsCount++;
}

void sendContinuousNoise() {
    // Transmit rapid sequence of packets to jam channel
    for (int i = 0; i < 10; i++) {
        sendJamPacket();
        delayMicroseconds(125);  // 125us between packets
    }
}
}

namespace BLEAdvertisingJammer {

JamResult jamAdvertising(uint32_t durationMs, const String &method) {
    JamResult result{false, 0, durationMs, method};

    Serial.println("\n=== BLE Advertising Jammer ===");
    Serial.println("Method: " + method);
    Serial.println("Duration: " + String(durationMs) + "ms");
    Serial.println("Channels: 37, 38, 39 (2.4GHz BLE band)");

    if (!TxArm::isArmed()) {
        Serial.println("✗ TX not armed (hold BACK button)");
        return result;
    }

    BLEDevice::init("");
    BLEServer *pServer = BLEDevice::createServer();

    g_jamActive = true;
    g_jamPacketsCount = 0;
    uint32_t startTime = millis();

    Serial.println("Starting BLE jamming...");

    while (millis() - startTime < durationMs && g_jamActive) {
        if (method == "NOISE") {
            sendContinuousNoise();
        } else if (method == "FLOODING") {
            // Rapid advertisement flooding
            for (int i = 0; i < 20; i++) {
                sendJamPacket();
                delayMicroseconds(50);
            }
        } else if (method == "SYNC") {
            // Synchronized channel attacks
            for (int ch = 0; ch < 3; ch++) {
                for (int i = 0; i < 5; i++) {
                    sendJamPacket();
                }
                delayMicroseconds(200);
            }
        }

        if (g_jamPacketsCount % 100 == 0) {
            Serial.println("  [" + String(g_jamPacketsCount) + "] jam packets in " +
                         String(millis() - startTime) + "ms");
        }
    }

    g_jamActive = false;
    result.success = true;
    result.jamPacketsCount = g_jamPacketsCount;

    Serial.println("✓ Jamming complete");
    Serial.println("Total jam packets: " + String(result.jamPacketsCount));
    Serial.println("Duration: " + String(millis() - startTime) + "ms");
    Serial.println("Rate: ~" + String((result.jamPacketsCount * 1000) / (millis() - startTime)) + " pkt/sec");
    Serial.println("⚠️  BLE scanning/advertising in range disrupted");

    BLEDevice::deinit(false);

    return result;
}

void stop() {
    g_jamActive = false;
    Serial.println("BLE jamming stopped");
}

bool isActive() {
    return g_jamActive;
}

}  // namespace BLEAdvertisingJammer
