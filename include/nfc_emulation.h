#pragma once
#include <Arduino.h>

namespace NFCEmulation {

enum NFCType {
    MIFARE_CLASSIC,
    MIFARE_ULTRALIGHT,
    NTAG215,
    GENERIC_ISO14443A
};

struct EmulationResult {
    bool success;
    NFCType emulatedType;
    uint32_t emulationDurationMs;
    uint16_t readCount;      // Number of times card was read
    String lastReaderData;   // Data sent by reader
    String error;
};

struct NFCCard {
    uint8_t uid[7];
    uint8_t uidLen;
    NFCType type;
    String friendlyName;
};

// Emulate a classic Mifare card
EmulationResult emulateCard(const NFCCard &card, uint16_t timeoutMs = 30000);

// Emulate with custom UID
EmulationResult emulateCustomUID(const uint8_t *uid, uint8_t uidLen, uint16_t timeoutMs = 30000);

// Get pre-configured test cards
std::vector<NFCCard> getTestCards();

} // namespace NFCEmulation
