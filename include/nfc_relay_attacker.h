#pragma once
#include <Arduino.h>

namespace NFCRelayAttacker {

struct RelayedCard {
    String uid;
    String cardType;
    uint8_t sak;
    uint8_t atqa[2];
    uint32_t timestamp;
};

struct RelayResult {
    bool success;
    uint32_t cardsDetected;
    uint32_t relayedSuccessfully;
    uint32_t durationMs;
    String lastRelayedUID;
};

// Perform NFC relay attack - capture distant NFC card and relay locally
RelayResult relayNFCCard(uint32_t durationMs = 30000);

// Get list of relayed cards
const RelayedCard* getRelayedCards(uint32_t& outCount);

// Emulate captured card
struct EmulationResult {
    bool success;
    String emulatedUID;
    uint32_t authenticationsSuccessful;
    uint32_t durationMs;
};
EmulationResult emulateRelayedCard(const char* targetUID, uint32_t durationMs = 20000);

// Analyze relay distance and signal strength
struct RelayAnalysis {
    bool success;
    uint32_t distanceEstimateM;
    int8_t signalStrength;
    uint32_t relayDelayMs;
    bool vulnerableToRelay;
};
RelayAnalysis analyzeRelayVulnerability(uint32_t durationMs = 15000);

// Jam legitimate reader while relaying
struct JammingResult {
    bool success;
    uint32_t jamPacketsSent;
    uint32_t validTransactionsCaptured;
    uint32_t durationMs;
};
JammingResult jamReaderDuringRelay(uint32_t durationMs = 25000);

}  // namespace NFCRelayAttacker
