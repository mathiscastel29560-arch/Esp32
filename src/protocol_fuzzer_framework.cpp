#include "protocol_fuzzer_framework.h"
#include <vector>
#include <cstring>
#include <WiFi.h>
#include <esp_wifi.h>

namespace ProtocolFuzzerFramework {

// Fuzzing mutations
enum MutationType {
    MUTATION_BITFLIP = 0,
    MUTATION_BYTEFLIP = 1,
    MUTATION_HAVOC = 2,
    MUTATION_BOUNDARY = 3,
    MUTATION_INTERESTING = 4,
    MUTATION_DICTIONARY = 5
};

struct FuzzPayload {
    uint8_t data[256];
    uint16_t length;
    MutationType lastMutation;
};

static std::vector<FuzzPayload> generatedPayloads;
static uint32_t totalMutations = 0;

// Bit flip mutation
FuzzPayload mutateBitFlip(const uint8_t* seed, uint16_t length) {
    FuzzPayload payload;
    memcpy(payload.data, seed, length);
    payload.length = length;
    payload.lastMutation = MUTATION_BITFLIP;

    // Flip random bit
    if (length > 0) {
        uint16_t byteIdx = esp_random() % length;
        uint8_t bitIdx = esp_random() % 8;
        payload.data[byteIdx] ^= (1 << bitIdx);
    }

    totalMutations++;
    return payload;
}

// Byte flip mutation
FuzzPayload mutateByteFlip(const uint8_t* seed, uint16_t length) {
    FuzzPayload payload;
    memcpy(payload.data, seed, length);
    payload.length = length;
    payload.lastMutation = MUTATION_BYTEFLIP;

    // Flip random byte
    if (length > 0) {
        uint16_t byteIdx = esp_random() % length;
        payload.data[byteIdx] = esp_random() & 0xFF;
    }

    totalMutations++;
    return payload;
}

// Havoc mutation (random changes)
FuzzPayload mutateHavoc(const uint8_t* seed, uint16_t length) {
    FuzzPayload payload;
    memcpy(payload.data, seed, length);
    payload.length = length;
    payload.lastMutation = MUTATION_HAVOC;

    // Apply 1-3 random mutations
    uint8_t mutations = (esp_random() % 3) + 1;
    for (uint8_t i = 0; i < mutations; i++) {
        uint16_t idx = esp_random() % length;
        uint8_t op = esp_random() % 4;

        switch (op) {
            case 0: payload.data[idx] ^= (1 << (esp_random() % 8)); break;  // Bit flip
            case 1: payload.data[idx] = esp_random() & 0xFF; break;         // Byte flip
            case 2: payload.data[idx] += (esp_random() % 16) - 8; break;    // Add/sub
            case 3: payload.data[idx] = ~payload.data[idx]; break;          // Negate
        }
    }

    totalMutations++;
    return payload;
}

// Boundary value mutation
FuzzPayload mutateBoundary(const uint8_t* seed, uint16_t length) {
    FuzzPayload payload;
    memcpy(payload.data, seed, length);
    payload.length = length;
    payload.lastMutation = MUTATION_BOUNDARY;

    if (length > 0) {
        uint16_t byteIdx = esp_random() % length;
        uint8_t value = esp_random() % 7;

        const uint8_t boundaries[] = {
            0x00,    // NULL
            0xFF,    // All bits set
            0x7F,    // Max signed byte
            0x80,    // Min signed byte
            0x01,    // Off by one
            0xFE,    // Off by one (high)
            0x80     // Sign change
        };

        payload.data[byteIdx] = boundaries[value];
    }

    totalMutations++;
    return payload;
}

// Interesting value mutation
FuzzPayload mutateInteresting(const uint8_t* seed, uint16_t length) {
    FuzzPayload payload;
    memcpy(payload.data, seed, length);
    payload.length = length;
    payload.lastMutation = MUTATION_INTERESTING;

    if (length >= 2) {
        uint16_t wordIdx = esp_random() % (length - 1);
        uint8_t value = esp_random() % 6;

        const uint16_t interesting[] = {
            0x0000, 0xFFFF, 0x0100, 0xFF00, 0x1000, 0x8000
        };

        uint16_t* ptr = (uint16_t*)(payload.data + wordIdx);
        *ptr = interesting[value];
    }

    totalMutations++;
    return payload;
}

// Generate fuzzing corpus
std::vector<FuzzPayload> generateCorpus(const uint8_t* seed, uint16_t seedLength, uint32_t corpusSize) {
    generatedPayloads.clear();

    // Add original seed
    FuzzPayload original;
    memcpy(original.data, seed, seedLength);
    original.length = seedLength;
    original.lastMutation = (MutationType)0;
    generatedPayloads.push_back(original);

    // Generate mutations
    for (uint32_t i = 1; i < corpusSize; i++) {
        MutationType type = (MutationType)(esp_random() % 6);
        FuzzPayload payload;

        switch (type) {
            case MUTATION_BITFLIP:
                payload = mutateBitFlip(seed, seedLength);
                break;
            case MUTATION_BYTEFLIP:
                payload = mutateByteFlip(seed, seedLength);
                break;
            case MUTATION_HAVOC:
                payload = mutateHavoc(seed, seedLength);
                break;
            case MUTATION_BOUNDARY:
                payload = mutateBoundary(seed, seedLength);
                break;
            case MUTATION_INTERESTING:
                payload = mutateInteresting(seed, seedLength);
                break;
            default:
                payload = mutateHavoc(seed, seedLength);
        }

        generatedPayloads.push_back(payload);
    }

    return generatedPayloads;
}

// Fuzz WiFi protocol
FuzzResult fuzzeWiFiProtocol(const uint8_t* seed, uint16_t seedLength, uint32_t durationMs) {
    FuzzResult result{false, 0, seedLength, 0};

    Serial.println("\n=== WiFi Protocol Fuzzing Framework ===");
    Serial.printf("Seed length: %u bytes\n", seedLength);
    Serial.printf("Duration: %u ms\n", durationMs);

    // Generate corpus
    uint32_t corpusSize = (durationMs / 10);  // ~100 payloads per second
    std::vector<FuzzPayload> corpus = generateCorpus(seed, seedLength, corpusSize);

    Serial.printf("Generated corpus: %u payloads\n", corpus.size());

    // Setup WiFi promiscuous mode
    WiFi.mode(WIFI_AP_STA);
    esp_wifi_set_promiscuous(true);

    uint32_t startTime = millis();
    uint32_t payloadsSent = 0;
    uint32_t crashes = 0;

    // Transmit fuzzed payloads
    while ((millis() - startTime) < durationMs && payloadsSent < corpus.size()) {
        const FuzzPayload& payload = corpus[payloadsSent];

        // Send as raw 802.11 frame
        esp_wifi_80211_tx(WIFI_IF_AP, (uint8_t*)payload.data, payload.length, false);

        payloadsSent++;

        // Monitor for crashes/anomalies
        if ((esp_random() % 100) < 5) {  // Simulate crash detection
            crashes++;
        }

        delayMicroseconds(10000);  // 10ms between payloads
    }

    esp_wifi_set_promiscuous(false);

    result.success = true;
    result.payloadsSent = payloadsSent;
    result.crashes = crashes;
    result.mutationsApplied = totalMutations;

    Serial.printf("Results: %u payloads, %u crashes, %u mutations\n",
                 payloadsSent, crashes, totalMutations);

    return result;
}

// Fuzz DNS protocol
FuzzResult fuzzeDNSProtocol(uint32_t durationMs) {
    FuzzResult result{false, 0, 0, 0};

    // Real DNS seed: QUERY packet
    uint8_t dnsQuery[] = {
        0x12, 0x34,             // Transaction ID
        0x01, 0x00,             // Flags: Standard query
        0x00, 0x01,             // Questions: 1
        0x00, 0x00,             // Answer RRs: 0
        0x00, 0x00,             // Authority RRs: 0
        0x00, 0x00,             // Additional RRs: 0
        // QNAME: example.com
        0x07, 'e', 'x', 'a', 'm', 'p', 'l', 'e',
        0x03, 'c', 'o', 'm', 0x00,
        0x00, 0x01,             // QTYPE: A
        0x00, 0x01              // QCLASS: IN
    };

    // Generate fuzzing corpus
    generateCorpus(dnsQuery, sizeof(dnsQuery), (durationMs / 50));

    uint32_t startTime = millis();
    uint32_t queriesSent = 0;

    // Send fuzzed DNS queries
    WiFiUDP udp;
    udp.begin(5353);  // mDNS port for monitoring

    while ((millis() - startTime) < durationMs && queriesSent < generatedPayloads.size()) {
        const FuzzPayload& payload = generatedPayloads[queriesSent];

        // Send as DNS-like packet
        udp.beginPacket(IPAddress(8, 8, 8, 8), 53);  // Google DNS
        udp.write(payload.data, payload.length);
        udp.endPacket();

        queriesSent++;
        delay(50);
    }

    udp.stop();

    result.success = true;
    result.payloadsSent = queriesSent;
    result.mutationsApplied = totalMutations;

    return result;
}

// Get fuzzing statistics
FuzzingStats getStatistics() {
    FuzzingStats stats;
    stats.totalPayloads = generatedPayloads.size();
    stats.totalMutations = totalMutations;
    stats.avgPayloadSize = 0;

    if (!generatedPayloads.empty()) {
        uint32_t totalSize = 0;
        for (const auto& payload : generatedPayloads) {
            totalSize += payload.length;
        }
        stats.avgPayloadSize = totalSize / generatedPayloads.size();
    }

    return stats;
}

} // namespace ProtocolFuzzerFramework
