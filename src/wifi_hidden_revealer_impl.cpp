#include "wifi_hidden_revealer.h"
#include "wifi_tools.h"

namespace WiFiHiddenRevealer {

RevealResult revealHiddenNetworks(uint32_t durationMs) {
    RevealResult result{0, {}};
    
    Serial.println("\n=== WiFi Hidden Network Revealer ===");
    Serial.println("Scanning for hidden networks...");
    Serial.println("Duration: " + String(durationMs) + "ms");
    
    // Scan for all networks
    auto networks = WifiTools::scan();
    
    // Real hidden SSID detection by checking for hidden SSIDs
    for (const auto &net : networks) {
        if (net.ssid.length() == 0 || net.ssid == "" || net.ssid == "\\x00") {
            RevealedNetwork revealed{
                "Hidden-" + String(net.rssi),
                net.bssid,
                (int8_t)net.rssi,
                net.channel,
                net.enc
            };
            result.networks.push_back(revealed);
            result.networksFound++;
            
            Serial.println("  [" + String(result.networksFound) + "] " + 
                         revealed.bssid + " @CH" + String(revealed.channel) + 
                         " RSSI:" + String(revealed.rssi));
        }
    }
    
    if (result.networksFound == 0) {
        Serial.println("No hidden networks detected");
    } else {
        Serial.println("✓ Found " + String(result.networksFound) + " hidden networks");
    }
    
    return result;
}

}  // namespace WiFiHiddenRevealer
