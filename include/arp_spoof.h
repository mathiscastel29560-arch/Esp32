#pragma once
#include <Arduino.h>
namespace ARPSpoof {
struct SpoofResult { bool active; String targetIP; String gatewayIP; uint32_t packetsSent; };
SpoofResult startMITM(const String &targetIP, const String &gatewayIP, uint16_t timeoutMs = 60000);
void stop();
} // namespace ARPSpoof
