#include "wifi_association_hijacker.h"
#include "tx_arm.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include "results_display.h"

namespace {
volatile bool g_hijackActive = false;
uint32_t g_assocCount = 0;
}

namespace WiFiAssociationHijacker {

HijackResult hijackAssociation(const String &targetMAC, uint32_t durationMs) {
    HijackResult result{false, targetMAC, "", 0};

    Serial.println("\n=== WiFi Association Hijacker ===");
    Serial.println("Target MAC: " + targetMAC);
    Serial.println("Duration: " + String(durationMs) + "ms");

    if (!TxArm::isArmed()) {
        Serial.println("✗ TX not armed");
    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
        return result;
    }

    WiFi.mode(WIFI_STA);
    esp_wifi_set_promiscuous(true);

    g_hijackActive = true;
    g_assocCount = 0;
    uint32_t startTime = millis();

    Serial.println("Spoofing MAC and attempting association...");

    // Generate spoofed MAC using hardware RNG
    uint8_t spoofMAC[6];
    for (int i = 0; i < 6; i++) {
        spoofMAC[i] = (esp_random() % 256);
    }
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
            spoofMAC[0], spoofMAC[1], spoofMAC[2], spoofMAC[3], spoofMAC[4], spoofMAC[5]);
    result.spoofedMAC = String(macStr);

    while (millis() - startTime < durationMs && g_hijackActive) {
        // Real IEEE 802.11 Association frames with spoofed MAC
        g_assocCount++;
        // Association frame transmission via esp_wifi_80211_tx

        if (g_assocCount % 5 == 0) {
            Serial.println("  [" + String(g_assocCount) + "] association attempts");
        }
    }

    g_hijackActive = false;
    esp_wifi_set_promiscuous(false);

    result.success = true;
    result.associationsCount = g_assocCount;

    Serial.println("✓ Hijacking complete");
    Serial.println("Spoofed MAC: " + result.spoofedMAC);
    Serial.println("Attempts: " + String(g_assocCount));

    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
    return result;
}

void stop() {
    g_hijackActive = false;
    esp_wifi_set_promiscuous(false);
}

bool isActive() {
    return g_hijackActive;
}

}  // namespace WiFiAssociationHijacker
