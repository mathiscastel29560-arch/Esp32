#pragma once
#include <Arduino.h>

namespace MifareClassic {

struct CardData {
    String uid;
    uint8_t sectors;
    String cardType;  // MIFARE Classic 1K, 4K
    bool locked;
};

struct ReadResult {
    bool success;
    String sectorData;
    uint32_t durationMs;
};

// Read Mifare Classic card data
ReadResult readMifareCard(uint32_t durationMs = 15000);

// Attack: Key recovery via hardcoded keys
struct KeyRecoveryResult {
    bool success;
    String keyFound;
    uint32_t attemptCount;
    uint32_t durationMs;
};
KeyRecoveryResult recoverMifareKeys(uint32_t durationMs = 30000);

// Attack: Clone Mifare card
struct CloneResult {
    bool success;
    String sourceUid;
    String clonedUid;
    uint32_t durationMs;
};
CloneResult cloneMifareCard(const char* sourceUid, uint32_t durationMs = 20000);

}  // namespace MifareClassic
