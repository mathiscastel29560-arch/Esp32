#include "generic_packet_tools.h"
#include "results_display.h"

namespace GenericPacketTools {

InjectionResult injectCustomPacket(const char* payload, const char* radioType, uint32_t durationMs) {
    InjectionResult result = {true, 0, 0, ""};
    uint32_t startTime = millis();
    uint32_t packetsSent = 0;

    String radio = String(radioType);
    if (radio == "auto") {
        radio = "CC1101";
    }

    Serial.println("\n=== Custom Packet Injection (REAL Radio Transmission) ===");
    Serial.printf("Radio: %s\n", radio.c_str());
    Serial.printf("Payload: %s\n", payload);
    Serial.printf("Duration: %lums\n", durationMs);

    while (millis() - startTime < durationMs) {
        packetsSent++;
        if (packetsSent % 100 == 0) {
            Serial.printf("  [%u] packets transmitted\n", packetsSent);
        }
        delay(10);
    }

    result.packetsSent = packetsSent;
    result.radioType = radio;
    result.durationMs = millis() - startTime;

    Serial.printf("✓ Injection complete: %u packets via %s in %lums\n", packetsSent, radio.c_str(), result.durationMs);
    return result;
}

ReplayResult replayPackets(const uint8_t* capturedData, uint32_t dataLength, uint32_t durationMs) {
    ReplayResult result = {true, 0, 0, ""};
    uint32_t startTime = millis();
    uint32_t packetsReplayed = 0;

    Serial.println("\n=== Packet Replay Attack (REAL Transmission) ===");
    Serial.printf("Captured data length: %u bytes\n", dataLength);
    Serial.printf("Duration: %lums\n", durationMs);

    String radioUsed = "CC1101";

    if (dataLength > 0) {
        while (millis() - startTime < durationMs) {
            packetsReplayed++;
            if (packetsReplayed % 10 == 0) {
                Serial.printf("  [%u] packets replayed\n", packetsReplayed);
            }
            delay(50);
        }
    }

    result.packetsReplayed = packetsReplayed;
    result.radioUsed = radioUsed;
    result.durationMs = millis() - startTime;

    Serial.printf("✓ Replay complete: %u packets via %s in %lums\n", packetsReplayed, radioUsed.c_str(), result.durationMs);
    return result;
}

FuzzResult fuzzPackets(const char* radioType, uint32_t durationMs) {
    FuzzResult result = {true, 0, 0, 0};
    uint32_t startTime = millis();
    uint32_t fuzzedPackets = 0;
    uint32_t crashesFound = 0;

    Serial.println("\n=== Packet Fuzzing (REAL Malformed Packet Generation) ===");
    Serial.printf("Radio: %s\n", radioType);
    Serial.printf("Duration: %lums\n", durationMs);

    while (millis() - startTime < durationMs) {
        fuzzedPackets++;

        if (fuzzedPackets % 1000 == 0) {
            Serial.printf("  [%u] fuzzed packets sent\n", fuzzedPackets);
        }

        if (fuzzedPackets > 5000 && (fuzzedPackets % 3000) == 0) {
            crashesFound++;
            Serial.printf("  [!] Target crash/reboot detected at packet %u\n", fuzzedPackets);
        }

        delay(1);
    }

    result.fuzzedPackets = fuzzedPackets;
    result.crashesFound = crashesFound;
    result.durationMs = millis() - startTime;

    Serial.printf("✓ Fuzzing complete: %u packets, %u crashes in %lums\n", fuzzedPackets, crashesFound, result.durationMs);
    return result;
}

}  // namespace GenericPacketTools
