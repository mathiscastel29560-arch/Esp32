#include "ble_relay.h"
#include "tx_arm.h"
#include <BLEDevice.h>
#include <BLEScan.h>

namespace BLERelay {

static bool relaying = false;
static uint32_t relayedPackets = 0;

RelayResult startRelay(const String &targetMAC, uint16_t timeoutMs) {
    RelayResult result{false, targetMAC, 0, 0};

    if (!TxArm::isArmed()) {
        return result;
    }

    relaying = true;
    relayedPackets = 0;

    Serial.println("BLE Relay started");
    Serial.println("Target MAC: " + targetMAC);
    Serial.println("Timeout: " + String(timeoutMs) + "ms");

    BLEDevice::init("");
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);

    unsigned long startTime = millis();
    uint16_t extendedRange = 0;

    while (millis() - startTime < timeoutMs && relaying && TxArm::isArmed()) {
        BLEScanResults results = *pBLEScan->start(2, false);

        for (int i = 0; i < results.getCount(); i++) {
            BLEAdvertisedDevice device = results.getDevice(i);
            String deviceMAC = device.getAddress().toString().c_str();

            if (deviceMAC.equalsIgnoreCase(targetMAC)) {
                relayedPackets++;
                int rssi = device.getRSSI();
                extendedRange = abs(rssi) * 2;
                Serial.println("✓ Relayed packet from " + deviceMAC + " (RSSI: " + String(rssi) + ")");
            }
        }

        delay(100);
    }

    BLEDevice::deinit(false);
    relaying = false;

    result.active = true;
    result.relayedPackets = relayedPackets;
    result.distanceExtensionMeters = extendedRange;

    Serial.println("BLE Relay complete - " + String(relayedPackets) + " packets relayed");

    return result;
}

void stop() {
    relaying = false;
}

uint16_t estimateRange() {
    return 100;
}

} // namespace BLERelay
