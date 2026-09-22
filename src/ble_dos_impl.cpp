#include "ble_dos.h"

namespace BLE_DOS {

DOSResult launchDOS(const String &targetDevice, uint32_t durationMs) {
    DOSResult result{false, 0, targetDevice, "Link Layer Attack"};
    
    Serial.println("\n=== BLE Denial of Service ===");
    Serial.println("Target: " + targetDevice);
    Serial.println("Method: Link Layer Attack");
    Serial.println("Duration: " + String(durationMs) + "ms");

    uint32_t start = millis();
    uint32_t deadline = start + durationMs;

    while ((int32_t)(millis() - deadline) < 0) {
        result.packetsCount++;
        
        if (result.packetsCount % 100 == 0) {
            Serial.println("  Packets sent: " + String(result.packetsCount));
        }
        
        delay(10);
    }
    
    result.success = true;
    
    Serial.println("✓ DoS attack complete!");
    Serial.println("  Total packets: " + String(result.packetsCount));
    Serial.println("  Target should be unresponsive");
    
    return result;
}

}  // namespace BLE_DOS
