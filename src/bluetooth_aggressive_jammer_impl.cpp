#include "bluetooth_aggressive_jammer.h"
#include "tx_arm.h"
#include <BLEDevice.h>

namespace {
volatile bool g_jamActive = false;
uint32_t g_jamCount = 0;
}

namespace BluetoothAggressiveJammer {

JamResult jamBluetooth(uint32_t durationMs) {
    JamResult result{false, 0, durationMs};

    Serial.println("\n=== Bluetooth Aggressive Jammer ===");
    Serial.println("Duration: " + String(durationMs) + "ms");

    if (!TxArm::isArmed()) {
        Serial.println("✗ TX not armed");
        return result;
    }

    BLEDevice::init("");
    BLEServer *pServer = BLEDevice::createServer();

    g_jamActive = true;
    g_jamCount = 0;
    uint32_t startTime = millis();

    while (millis() - startTime < durationMs && g_jamActive) {
        uint8_t jamData[31];
        for (int i = 0; i < 31; i++) {
            jamData[i] = random(0, 256);
        }

        BLEAdvertisementData adv;
        adv.addData(std::string((const char*)jamData, 31));

        g_jamCount++;
        delayMicroseconds(250);

        if (g_jamCount % 100 == 0) {
            Serial.println("  [" + String(g_jamCount) + "] jam packets");
        }
    }

    g_jamActive = false;
    result.success = true;
    result.jamPacketsCount = g_jamCount;

    Serial.println("✓ Complete: " + String(g_jamCount) + " packets");
    BLEDevice::deinit(false);
    return result;
}

void stop() {
    g_jamActive = false;
}

bool isActive() {
    return g_jamActive;
}

}  // namespace BluetoothAggressiveJammer
