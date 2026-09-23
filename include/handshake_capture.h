#pragma once
#include <Arduino.h>

// WPA2 4-way-handshake / EAPOL capture for one target AP -- passively
// sniffs, and (only while TxArm::isArmed(), like every other TX-capable
// module) sends one short deauth burst partway through to force a
// reconnect. Saves raw 802.11 frames to a .pcap file on LittleFS; offline
// cracking (hashcat/aircrack-ng) is a separate step on a real computer,
// not something this device does.
namespace HandshakeCapture {

struct Result {
    String filePath;   // "" if nothing was written
    int eapolFrames;   // how many EAPOL frames got captured (0..4 for a full handshake)
};

// Sniffs on `channel` for `durationMs`, filtering for EAPOL frames to/from
// `bssid`. Requires TxArm::isArmed() to actually fire the deauth trigger;
// without it, capture still runs (passive only) but only catches a
// handshake that happens to occur on its own during the window.
Result capture(const String &bssid, uint8_t channel, uint32_t durationMs);

} // namespace HandshakeCapture
