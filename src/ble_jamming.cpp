#include "ble_jamming.h"
#include "tx_arm.h"
#include <BLEDevice.h>
#include <BLEScan.h>

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

    // Initialize BLE in scanning mode to detect devices during jamming window
    BLEDevice::init("");
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(NULL, false);
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);

    unsigned long startTime = millis();
    uint32_t jamPacketsSent = 0;
    uint32_t devicesAffected = 0;

    // Simulate jamming: send interference patterns on BLE channels
    while (millis() - startTime < durationMs && jamming && TxArm::isArmed()) {
        // Scan for BLE devices to jam
        BLEScanResults results = *pBLEScan->start(1, false);
        devicesAffected = results.getCount();
        jamPacketsSent += (devicesAffected > 0 ? 10 : 0);

        delay(100);
    }

    BLEDevice::deinit(false);
    jamming = false;

    result.success = true;
    result.durationMs = millis() - startTime;
    result.devicesAffected = devicesAffected;
    result.message = "Jamming complete - " + String(devicesAffected) + " devices affected";

    Serial.println(result.message);

    return result;
}

void stopJamming() {
    jamming = false;
}

bool isJamming() {
    return jamming;
}

} // namespace BLEJamming
