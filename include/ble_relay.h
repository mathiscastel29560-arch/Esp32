#pragma once
#include <Arduino.h>
namespace BLERelay {
struct RelayResult { bool active; String targetMAC; uint32_t relayedPackets; uint16_t distanceExtensionMeters; };
RelayResult startRelay(const String &targetMAC, uint16_t timeoutMs = 60000);
void stop();
uint16_t estimateRange();
} // namespace BLERelay
