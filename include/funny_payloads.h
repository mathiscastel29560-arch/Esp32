#pragma once
#include <Arduino.h>
#include <vector>

namespace FunnyPayloads {

struct FunnyPayload {
    String title;
    String description;
};

// Bad USB payloads drôles
std::vector<String> getFunnyBadUSBPayloads();

// WiFi SSID drôles
std::vector<String> getFunnyWiFiSSIDs();

// Messages DNS Spoof drôles
String getFunnyDNSMessage();

// Messages RFID clone drôles
String getFunnyRFIDMessage();

// Messages BLE drôles
String getFunnyBLEMessage();

// Chaos mode: lance tous les outils en même temps
struct ChaosResult {
    bool success;
    uint16_t toolsExecuted;
    String summary;
};

ChaosResult launchChaosMode();

} // namespace FunnyPayloads
