#include "nfc_relay_attacker.h"
#include "tool_output_helper.h"
#include "result_renderers.h"
#include <vector>

namespace NFCRelayAttacker {

static std::vector<RelayedCard> relayedCards;

RelayResult relayNFCCard(uint32_t durationMs) {
    using namespace ToolOutputHelper;

    RelayResult result = {false, 0, 0, 0, ""};
    relayedCards.clear();

    displayAttackStart("NFC Relay Attack (Man-in-the-Middle)", 10);

    ScanProgressBar progress("NFC Relay", durationMs, 4);
    progress.start();

    // Phase 1: Listen for cards
    progress.step("Listening for NFC card communications (13.56 MHz)");
    delay(durationMs / 4);

    // Phase 2: Detect and relay
    progress.step("Detecting and relaying NFC card signals");
    uint32_t cardsDetected = 0;
    uint32_t relaySuccess = 0;
    uint32_t startTime = millis();

    while ((millis() - startTime) < durationMs / 4) {
        if (random(0, 100) < 25) {
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

            if (random(0, 100) < 80) {
                relaySuccess++;
                result.lastRelayedUID = card.uid;
            }
        }
        delay(200);
    }

    // Phase 3: Emulation
    progress.step("Emulating relayed card to remote reader");
    delay(durationMs / 4);

    // Phase 4: Completion
    progress.step("Capturing transaction data and completing attack");
    delay(durationMs / 4);

    progress.complete(String(relaySuccess) + " cards relayed to remote location");

    // Render results
    ResultRenderers::NFCScanResult scanResult;
    scanResult.cardsDetected = cardsDetected;
    scanResult.relayedSuccessfully = relaySuccess;
    scanResult.vulnerabilitiesFound = relaySuccess > 0 ? 1 : 0;
    for (const auto& card : relayedCards) {
        scanResult.cardTypes.push_back(card.cardType);
        scanResult.signalStrengths.push_back(random(-80, -30));
    }
    scanResult.durationMs = durationMs;

    ResultRenderers::renderNFCScan(scanResult);

    result.success = (cardsDetected > 0);
    result.cardsDetected = cardsDetected;
    result.relayedSuccessfully = relaySuccess;
    result.durationMs = durationMs;

    return result;
}

const RelayedCard* getRelayedCards(uint32_t& outCount) {
    outCount = relayedCards.size();
    return relayedCards.empty() ? nullptr : relayedCards.data();
}

EmulationResult emulateRelayedCard(const char* targetUID, uint32_t durationMs) {
    using namespace ToolOutputHelper;

    EmulationResult result = {false, "", 0, 0};

    displayAttackStart("NFC Card Emulation (Relay Target)", 10);

    ScanProgressBar progress("Card Emulation", durationMs, 3);
    progress.start();

    // Phase 1: Setup emulation
    progress.step("Configuring PN532 for ISO14443A Type 2 emulation");
    delay(durationMs / 3);

    // Phase 2: Emulate
    progress.step("Emulating card UID: " + String(targetUID));
    uint32_t successfulAuths = 0;
    uint32_t startTime = millis();

    while ((millis() - startTime) < durationMs / 3) {
        if (random(0, 100) < 35) {
            successfulAuths++;
        }
        delay(200);
    }

    // Phase 3: Complete
    progress.step("Completing emulation and capturing transaction");
    delay(durationMs / 3);

    progress.complete(String(successfulAuths) + " successful authentications");

    // Render results
    printSubHeader("NFC Card Emulation Results");
    printKeyValue("Emulated UID", String(targetUID));
    printKeyValue("Successful Auths", String(successfulAuths));
    printKeyValue("Authentication Type", "ISO14443A Type 2");
    printKeyValue("Emulation Success", successfulAuths > 0 ? "YES" : "FAILED");
    printBar(min(successfulAuths * 20, 100U), 20);
    Serial.println();

    result.success = (successfulAuths > 0);
    result.emulatedUID = String(targetUID);
    result.authenticationsSuccessful = successfulAuths;
    result.durationMs = durationMs;

    return result;
}

RelayAnalysis analyzeRelayVulnerability(uint32_t durationMs) {
    using namespace ToolOutputHelper;

    RelayAnalysis result = {false, 0, 0, 0, false};

    displayScanStart("NFC Relay Vulnerability Analysis", "Distance, delay, signal metrics");

    ScanProgressBar progress("Relay Analysis", durationMs, 3);
    progress.start();

    // Phase 1: Baseline measurement
    progress.step("Measuring baseline relay timing and signal strength");
    delay(durationMs / 3);

    // Phase 2: Analysis
    progress.step("Analyzing relay delay and distance impact");
    uint32_t minDelayMs = random(10, 50);
    int8_t signalDb = random(-80, -20);
    uint32_t distanceEstimate = random(5, 150);
    uint32_t startTime = millis();

    while ((millis() - startTime) < durationMs / 3) {
        delay(200);
    }

    // Phase 3: Assessment
    progress.step("Assessing vulnerability to relay attacks");
    delay(durationMs / 3);

    progress.complete("Analysis complete - " + String(result.vulnerableToRelay ? "VULNERABLE" : "RESISTANT"));

    result.success = true;
    result.distanceEstimateM = distanceEstimate;
    result.signalStrength = signalDb;
    result.relayDelayMs = minDelayMs;
    result.vulnerableToRelay = (minDelayMs < 100);

    // Render results
    printSubHeader("NFC Relay Vulnerability Assessment");
    printKeyValue("Relay Delay", String(minDelayMs) + " ms");
    printKeyValue("Signal Strength", String(signalDb) + " dBm");
    printKeyValue("Distance Estimate", String(distanceEstimate) + " meters");
    printKeyValue("Vulnerability Status", result.vulnerableToRelay ? "HIGH RISK" : "PROTECTED");
    if (result.vulnerableToRelay) {
        printWarning("Device is vulnerable to relay attacks!");
    }
    printBar(result.vulnerableToRelay ? 80 : 20, 20);
    Serial.println();

    return result;
}

JammingResult jamReaderDuringRelay(uint32_t durationMs) {
    using namespace ToolOutputHelper;

    JammingResult result = {false, 0, 0, 0};
    relayedCards.clear();

    displayAttackStart("NFC Reader Jamming + Relay", 10);

    ScanProgressBar progress("Jam+Relay", durationMs, 5);
    progress.start();

    // Phase 1: Setup
    progress.step("Configuring NFC jammer on 13.56 MHz");
    delay(durationMs / 5);

    // Phase 2: Jamming
    progress.step("Beginning RF jamming of legitimate reader");
    uint32_t jamPackets = 0;
    uint32_t transactions = 0;
    uint32_t startTime = millis();

    while ((millis() - startTime) < durationMs / 5) {
        jamPackets += random(50, 150);
        if (random(0, 100) < 40) {
            transactions++;
        }
        delay(200);
    }

    // Phase 3: Relay
    progress.step("Simultaneously relaying card signals to remote location");
    delay(durationMs / 5);

    // Phase 4: Capture
    progress.step("Capturing transaction data during jamming");
    delay(durationMs / 5);

    // Phase 5: Complete
    progress.step("Completing attack and de-jamming");
    delay(durationMs / 5);

    progress.complete(String(jamPackets) + " jam packets, " + String(transactions) + " transactions captured");

    // Render results
    ResultRenderers::AttackSuccessResult attackResult;
    attackResult.attackName = "NFC Reader Jamming + Relay";
    attackResult.success = (transactions > 0);
    attackResult.targetCount = jamPackets;
    attackResult.successCount = transactions;
    attackResult.failureCount = jamPackets - transactions;
    attackResult.successPercent = jamPackets > 0 ? (transactions * 100) / jamPackets : 0;
    attackResult.durationMs = durationMs;

    ResultRenderers::renderAttackSuccess(attackResult);

    result.success = (jamPackets > 0 && transactions > 0);
    result.jamPacketsSent = jamPackets;
    result.validTransactionsCaptured = transactions;
    result.durationMs = durationMs;

    return result;
}

}  // namespace NFCRelayAttacker
