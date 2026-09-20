#pragma once
#include <Arduino.h>

// Targeted 802.11 deauthentication — for testing a specific AP/client you
// own or are otherwise authorized to test (e.g. checking whether a WIDS
// flags it, or how a device handles reassociation). Every call requires an
// explicit BSSID, so this cannot be pointed at "everything nearby" by
// accident. Requires the hardware safety switch to be armed.
namespace Deauth {

// client = "" or "FF:FF:FF:FF:FF:FF" deauths every station on that BSSID.
// Returns false (and transmits nothing) if the safety switch is off or the
// BSSID doesn't parse.
bool send(const String &bssid, const String &client, uint8_t channel, uint16_t frames = 30);

} // namespace Deauth
