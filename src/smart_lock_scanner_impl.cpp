#include "smart_lock_scanner.h"
#include "wifi_tools.h"

namespace SmartLockScanner {

ScanResult scanSmartLocks(uint32_t durationMs) {
    ScanResult result{0, {}};
    
    Serial.println("\n=== Smart Lock Exploit Scanner ===");
    Serial.println("Scanning for smart locks...");
    
    const char* lock_patterns[] = {"lock", "august", "yale", "kevo", "nuki", "danalock", "u-bolt"};
    
    auto networks = WifiTools::scan();
    
    for (const auto &net : networks) {
        String ssid_lower = net.ssid;
        ssid_lower.toLowerCase();
        
        for (const char* pattern : lock_patterns) {
            if (ssid_lower.indexOf(pattern) >= 0) {
                LockDetection detection{pattern, net.ssid, (int8_t)net.rssi, "WiFi/BLE"};
                result.detections.push_back(detection);
                result.locksFound++;
                
                Serial.println("  [LOCK] " + String(pattern) + " @ " + net.ssid + 
                             " RSSI:" + String(net.rssi));
                break;
            }
        }
    }
    
    if (result.locksFound == 0) {
        Serial.println("No smart locks detected");
    } else {
        Serial.println("✓ Found " + String(result.locksFound) + " smart locks");
    }
    
    return result;
}

}  // namespace SmartLockScanner
