#include "ble_jamming.h"
#include "tx_arm.h"
#include <NimBLEDevice.h>

namespace BLEJamming {

static bool jamming = false;
static uint32_t jamDurationMs = 0;

JamResult startJamming(uint32_t durationMs, uint8_t powerLevel) {
    JamResult result{false, 0, 0, "⚠️ BLE jamming requires TX arming"};

    if (!TxArm::isArmed()) {
        return result;
    }

    jamming = true;
    jamDurationMs = durationMs;

    Serial.println("BLE Jamming started");
    Serial.println("Duration: " + String(durationMs) + "ms");
    Serial.println("Power level: " + String(powerLevel));

    // Initialize NimBLE for advertising (jamming)
    NimBLEDevice::init("");
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();

    unsigned long startTime = millis();
    uint32_t jamPacketsSent = 0;

    // Send interference patterns on BLE channels
    while (millis() - startTime < durationMs && jamming && TxArm::isArmed()) {
        // Send jamming advertisements to interfere with BLE communications
        uint8_t jamPayload[31];
        for (int i = 0; i < 31; i++) {
            jamPayload[i] = esp_random() % 256;
        }

        NimBLEAdvertisementData jamAdvData;
        jamAdvData.setFlags(0x06);
        jamAdvData.addData(std::string((const char *)jamPayload, 31));

        pAdvertising->setAdvertisementData(jamAdvData);
        pAdvertising->start(0, nullptr, nullptr);
        delayMicroseconds(500);
        pAdvertising->stop();
        jamPacketsSent++;

        delay(10);
    }

    NimBLEDevice::deinit();
    jamming = false;

    result.success = true;
    result.durationMs = millis() - startTime;
    result.powerLevel = powerLevel;
    result.error = "";

    Serial.println("✓ Jamming complete: " + String(jamPacketsSent) + " jam packets sent");

    return result;
}

void stopJamming() {
    jamming = false;
}

bool isJamming() {
    return jamming;
}

} // namespace BLEJamming
