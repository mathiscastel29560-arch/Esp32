#pragma once
#include <Arduino.h>

namespace WiFiAssociationHijacker {

struct HijackResult {
    bool success;
    String targetMAC;
    String spoofedMAC;
    uint32_t associationsCount;
};

// Hijack WiFi association - take a connected client's place
HijackResult hijackAssociation(const String &targetMAC, uint32_t durationMs = 10000);

void stop();
bool isActive();

}  // namespace WiFiAssociationHijacker
