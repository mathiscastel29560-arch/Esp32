#pragma once
#include <Arduino.h>
#include <vector>

// FEATURES.md Module 2 — connects as a GATT client to ONE device (an
// address you provide, from your own scan results), enumerates its
// services/characteristics, and flags common security smells. Every call
// targets exactly the address given; nothing here is broadcast or scoped
// more broadly. Use only on devices you own or are authorized to test.
namespace BleGattAudit {

struct CharFinding {
    String serviceUuid;
    String charUuid;
    bool readable = false;
    bool writable = false;
    bool notifiable = false;
    bool readableWithoutPairing = false;
    bool writableWithoutAuth = false;
};

struct AuditReport {
    bool connected = false;
    std::vector<CharFinding> findings;

    // Set by attempting to pair after the unauthenticated pass. If bonded
    // and encrypted but not authenticated, pairing was "Just Works" — no
    // protection against a man-in-the-middle.
    bool pairingAttempted = false;
    bool bonded = false;
    bool encrypted = false;
    bool authenticated = false;

    // Device Information Service (0x180A) fields that were readable.
    std::vector<String> deviceInfoLeaks;
};

// Scans for `timeoutSeconds` to find the device (needed to get its real
// address type), then connects and runs the audit.
AuditReport audit(const String &address, uint32_t scanTimeoutSeconds = 5);

} // namespace BleGattAudit
