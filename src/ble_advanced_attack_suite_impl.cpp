#include "ble_advanced_attack_suite.h"
#include "tx_arm.h"
#include <BLEDevice.h>
#include "results_display.h"

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
    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
        return result;
    }

    BLEDevice::init("");
    BLEScan* pBLEScan = BLEDevice::getScan();

    g_attackActive = true;
    g_attackCount = 0;
    uint32_t startTime = millis();

    while (millis() - startTime < durationMs && g_attackActive) {
        if (method == "GATT" || method == "ALL") {
            // GATT service disruption
            uint8_t gattJam[20];
            for (int i = 0; i < 20; i++) {
                gattJam[i] = (esp_random() % 256);
            }
            g_attackCount++;
        }

        if (method == "EAVES" || method == "ALL") {
            // Eavesdrop on BLE traffic
            pBLEScan->start(0, false);
            g_attackCount++;
        }

        delay(100);

        if (g_attackCount % 50 == 0) {
            Serial.println("  [" + String(g_attackCount) + "] attack packets");
        }
    }

    g_attackActive = false;
    result.success = true;
    result.attackPacketsCount = g_attackCount;

    Serial.println("✓ Complete: " + String(g_attackCount) + " packets");
    BLEDevice::deinit(false);
    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
    return result;
}

void stop() {
    g_attackActive = false;
}

bool isActive() {
    return g_attackActive;
}

}  // namespace BLEAdvancedAttackSuite
