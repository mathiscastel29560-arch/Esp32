#pragma once
#include <Arduino.h>

namespace RfidEmulator {

struct EmulationResult {
    bool success;
    String emulatedCardId;
    uint32_t durationMs;
    String cardType;  // HID, AWID, EM4100, T5577
};

// Emulate 125kHz RFID cards (HID, AWID, EM4100)
EmulationResult emulateRfidCard(const char* cardType = "HID", uint32_t durationMs = 10000);

// Attack: Brute force RFID card IDs
struct BruteforceResult {
    bool success;
    uint32_t validCardId;
    uint32_t attemptCount;
    uint32_t durationMs;
};
BruteforceResult bruteforceRfidCards(uint32_t durationMs = 30000);

// Attack: Clone RFID card
struct CloneResult {
    bool success;
    String sourceCardId;
    String clonedCardId;
    uint32_t durationMs;
};
CloneResult cloneRfidCard(const char* sourceCardId, uint32_t durationMs = 15000);

}  // namespace RfidEmulator
