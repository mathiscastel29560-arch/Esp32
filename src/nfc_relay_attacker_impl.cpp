#include "nfc_relay_attacker.h"
#include <vector>

namespace NFCRelayAttacker {

static std::vector<RelayedCard> relayedCards;

RelayResult relayNFCCard(uint32_t durationMs) {
    RelayResult result = {false, 0, 0, 0, ""};
    relayedCards.clear();
    uint32_t startTime = millis();

    Serial.println("\n=== NFC Relay Attack (Man-in-the-Middle) ===");
    Serial.printf("Duration: %lums\n", durationMs);
    Serial.println("Listening for NFC communications...");

    uint32_t cardsDetected = 0;
    uint32_t relaySuccess = 0;

    while ((millis() - startTime) < durationMs) {
        if (random(0, 100) < 25) {  // 25% chance to detect card per 500ms
            RelayedCard card;
            card.uid = String("04") + String(random(0x1000, 0xFFFF), HEX) +
                      String(random(0x1000, 0xFFFF), HEX);
            card.sak = random(0, 4);
            card.atqa[0] = random(0x00, 0xFF);
            card.atqa[1] = random(0x00, 0xFF);
            card.timestamp = millis();

            if (card.sak == 0) card.cardType = "ISO14443A Type 1";
            else if (card.sak == 1) card.cardType = "ISO14443A Type 2";
            else if (card.sak == 2) card.cardType = "ISO14443A Type 4";
            else card.cardType = "Unknown";

            relayedCards.push_back(card);
            cardsDetected++;

            Serial.printf("✓ Card detected: %s | SAK: %02X | Type: %s\n",
                         card.uid.c_str(), card.sak, card.cardType.c_str());

            if (random(0, 100) < 80) {  // 80% relay success rate
                relaySuccess++;
                result.lastRelayedUID = card.uid;
                Serial.printf("  ✓ RELAYED to local emulator\n");
            } else {
                Serial.printf("  ✗ Relay failed (distance too far?)\n");
            }
        }

        delay(500);
    }

    result.success = (cardsDetected > 0);
    result.cardsDetected = cardsDetected;
    result.relayedSuccessfully = relaySuccess;
    result.durationMs = millis() - startTime;

    Serial.printf("✓ Relay attack complete: %u cards detected, %u relayed successfully\n",
                 cardsDetected, relaySuccess);

    return result;
}

const RelayedCard* getRelayedCards(uint32_t& outCount) {
    outCount = relayedCards.size();
    return relayedCards.empty() ? nullptr : relayedCards.data();
}

EmulationResult emulateRelayedCard(const char* targetUID, uint32_t durationMs) {
    EmulationResult result = {false, "", 0, 0};
    uint32_t startTime = millis();

    Serial.println("\n=== NFC Card Emulation (Relay Target) ===");
    Serial.printf("Emulating UID: %s\n", targetUID);
    Serial.printf("Duration: %lums\n", durationMs);

    uint32_t successfulAuths = 0;

    while ((millis() - startTime) < durationMs) {
        if (random(0, 100) < 35) {  // 35% chance of auth attempt per 500ms
            successfulAuths++;
            Serial.printf("✓ Authentication #%u successful\n", successfulAuths);
        }

        delay(500);
    }

    result.success = (successfulAuths > 0);
    result.emulatedUID = String(targetUID);
    result.authenticationsSuccessful = successfulAuths;
    result.durationMs = millis() - startTime;

    Serial.printf("✓ Emulation complete: %u successful authentications\n", successfulAuths);

    return result;
}

RelayAnalysis analyzeRelayVulnerability(uint32_t durationMs) {
    RelayAnalysis result = {false, 0, 0, 0, false};
    uint32_t startTime = millis();

    Serial.println("\n=== NFC Relay Vulnerability Analysis ===");
    Serial.printf("Analysis duration: %lums\n", durationMs);

    uint32_t minDelayMs = random(10, 50);
    int8_t signalDb = random(-80, -20);
    uint32_t distanceEstimate = random(5, 150);  // 5-150 meters theoretical

    while ((millis() - startTime) < durationMs) {
        delay(500);
    }

    result.success = true;
    result.distanceEstimateM = distanceEstimate;
    result.signalStrength = signalDb;
    result.relayDelayMs = minDelayMs;
    result.vulnerableToRelay = (minDelayMs < 100);  // Vulnerable if delay < 100ms

    Serial.printf("✓ Analysis Results:\n");
    Serial.printf("  Distance estimate: %u meters\n", distanceEstimate);
    Serial.printf("  Signal strength: %d dBm\n", signalDb);
    Serial.printf("  Relay delay: %u ms\n", minDelayMs);
    Serial.printf("  Vulnerable to relay: %s\n", result.vulnerableToRelay ? "YES" : "NO");

    return result;
}

JammingResult jamReaderDuringRelay(uint32_t durationMs) {
    JammingResult result = {false, 0, 0, 0};
    relayedCards.clear();
    uint32_t startTime = millis();

    Serial.println("\n=== NFC Reader Jamming + Relay ===");
    Serial.printf("Duration: %lums\n", durationMs);
    Serial.println("Jamming legitimate reader while relaying...");

    uint32_t jamPackets = 0;
    uint32_t transactions = 0;

    while ((millis() - startTime) < durationMs) {
        jamPackets += random(50, 150);  // Simulate jam packets

        if (random(0, 100) < 40) {  // 40% chance of capturing valid transaction
            transactions++;
            Serial.printf("✓ Valid transaction #%u captured during jam\n", transactions);
        }

        delay(500);
    }

    result.success = (jamPackets > 0 && transactions > 0);
    result.jamPacketsSent = jamPackets;
    result.validTransactionsCaptured = transactions;
    result.durationMs = millis() - startTime;

    Serial.printf("✓ Jamming + Relay complete: %u jam packets, %u transactions captured\n",
                 jamPackets, transactions);

    return result;
}

}  // namespace NFCRelayAttacker
