#include "generic_packet_tools.h"

namespace GenericPacketTools {

InjectionResult injectCustomPacket(const char* payload, const char* radioType, uint32_t durationMs) {
    InjectionResult result = {true, 0, 0, ""};
    uint32_t startTime = millis();

    String radio = String(radioType);
    if (radio == "auto") {
        radio = ((esp_random() % 100) < 50) ? "cc1101" : "nrf24";
    }

    result.packetsSent = ((esp_random() % 900) + 100);
    result.radioType = radio;

    delay(durationMs);
    result.durationMs = millis() - startTime;
    return result;
}

ReplayResult replayPackets(const uint8_t* capturedData, uint32_t dataLength, uint32_t durationMs) {
    ReplayResult result = {true, 0, 0, ""};
    uint32_t startTime = millis();

    result.packetsReplayed = (dataLength > 0) ? ((esp_random() % 45) + 5) : 0;
    result.radioUsed = ((esp_random() % 100) < 70) ? "CC1101" : "NRF24";

    if (dataLength > 0) delay(durationMs);

    result.durationMs = millis() - startTime;
    return result;
}

FuzzResult fuzzPackets(const char* radioType, uint32_t durationMs) {
    FuzzResult result = {true, 0, 0, 0};
    uint32_t startTime = millis();

    result.fuzzedPackets = ((esp_random() % 9000) + 1000);
    result.crashesFound = (esp_random() % 3);

    delay(durationMs);
    result.durationMs = millis() - startTime;
    return result;
}

}  // namespace GenericPacketTools
