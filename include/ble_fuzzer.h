#pragma once
#include <Arduino.h>

// FEATURES.md Module 4 — targeted resilience test of ONE device you own,
// run in an isolated RF environment (Faraday bag, or minimal power at very
// short range). Connects to that single address and, over that one
// connection, sends oversized writes, writes to read-only characteristics,
// and rapid connect/disconnect cycles, then reports whether the target
// stayed responsive. Being connection-oriented to one address, it cannot
// affect any other device — unlike an advertising flood, which is why
// this is in scope while BLE-spam is not (see FEATURES.md).
namespace BleFuzzer {

struct FuzzReport {
    bool connected = false;
    int oversizedWritesAttempted = 0;
    int oversizedWritesAccepted = 0;
    int readOnlyWritesAttempted = 0;
    int readOnlyWritesAccepted = 0;
    int reconnectCyclesAttempted = 0;
    int reconnectCyclesFailed = 0;
    bool deviceUnresponsiveAtEnd = false;
};

FuzzReport fuzz(const String &address, uint32_t scanTimeoutSeconds = 5);

} // namespace BleFuzzer
