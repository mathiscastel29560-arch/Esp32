#pragma once
#include <Arduino.h>

namespace AdvancedWifiAttacks {

// KRACK Attack (Key Reinstallation Attack) - WPA2 exploit
struct KrackResult {
    bool success;
    uint32_t handshakesIntercepted;
    uint32_t keysRecovered;
    uint32_t durationMs;
};
KrackResult executeKrackAttack(uint32_t durationMs = 30000);

// Evil Twin + DHCP Starvation
struct EvilTwinResult {
    bool success;
    uint32_t clientsCaptured;
    uint32_t dhcpExhausted;
    uint32_t durationMs;
};
EvilTwinResult launchEvilTwinDhcp(const char* targetSsid, uint32_t durationMs = 30000);

// CTS/RTS Jamming - WiFi collision attack
struct JammingResult {
    bool success;
    uint32_t packetsJammed;
    uint32_t collisionsCreated;
    uint32_t durationMs;
};
JammingResult jamCtsRts(uint32_t durationMs = 20000);

// 802.11w PMF Bypass
struct PmfBypassResult {
    bool success;
    uint32_t attemptCount;
    uint32_t durationMs;
    String vulnerabilityFound;
};
PmfBypassResult bypassPmf(uint32_t durationMs = 25000);

// Force AP Downgrade WPA3 -> WPA2
struct DowngradeResult {
    bool success;
    uint32_t clientsDowngraded;
    uint32_t durationMs;
    String targetSsid;
};
DowngradeResult forceApDowngrade(const char* targetSsid, uint32_t durationMs = 20000);

}  // namespace AdvancedWifiAttacks
