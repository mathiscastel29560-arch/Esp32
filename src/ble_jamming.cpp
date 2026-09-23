#include "ble_jamming.h"
#include "tx_arm.h"
#include <NimBLEDevice.h>
#include <NimBLEAdvertising.h>

namespace BLEJamming {

static bool jamming = false;
static uint32_t jamPacketsSent = 0;
static NimBLEAdvertising* pAdvertising = nullptr;

JamResult startJamming(uint32_t durationMs, uint8_t powerLevel) {
    JamResult result{false, 0, 0, "BLE jamming requires TX arming"};

    if (!TxArm::isArmed()) {
        result.error = "✗ TX not armed (hold BACK button)";
        return result;
    }

    Serial.println("\n=== BLE Jamming (REAL Advertisement Injection) ===");
    Serial.println("Duration: " + String(durationMs) + "ms");
    Serial.println("Power level: " + String(powerLevel));
    Serial.println("Channels: 37, 38, 39 (2.4GHz BLE band)");

    NimBLEDevice::init("");
    NimBLEServer *pServer = NimBLEDevice::createServer();
    pAdvertising = NimBLEDevice::getAdvertising();

    pAdvertising->setAdvertisedDeviceCallbacks(nullptr);
    pAdvertising->setAdvertisementType(BLE_GAP_CONN_MODE_NON);
    pAdvertising->setTxPower(powerLevel);

    jamming = true;
    jamPacketsSent = 0;
    uint32_t startTime = millis();

    Serial.println("Transmitting malformed BLE advertisement packets...");

    while (millis() - startTime < durationMs && jamming && TxArm::isArmed()) {
        uint8_t jamPayload[31];

        for (int i = 0; i < 31; i++) {
            jamPayload[i] = random(0, 256);
        }

        NimBLEAdvertisementData advData;
        advData.addData(std::string((const char*)jamPayload, 31));

        pAdvertising->setAdvertisementData(advData);
        pAdvertising->start();

        delayMicroseconds(625);
        pAdvertising->stop();
        delayMicroseconds(625);

        jamPacketsSent++;

        if (jamPacketsSent % 100 == 0) {
            Serial.printf("  [%u] jam packets in %lums\n",
                         jamPacketsSent, millis() - startTime);
        }
    }

    jamming = false;
    pAdvertising->stop();
    NimBLEDevice::deinit(false);

    result.success = true;
    result.durationMs = millis() - startTime;
    result.powerLevel = powerLevel;
    result.error = "";

    Serial.printf("✓ BLE jamming complete: %u advertisement packets (%.1f pkt/sec)\n",
                 jamPacketsSent, (jamPacketsSent * 1000.0f) / result.durationMs);

    return result;
}

void stopJamming() {
    jamming = false;
    if (pAdvertising) {
        pAdvertising->stop();
    }
}

bool isJamming() {
    return jamming;
}

} // namespace BLEJamming
