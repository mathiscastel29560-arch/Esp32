#include "subghz_replay.h"

namespace SubGhzReplay {

CaptureResult capture(uint32_t frequencyMhz) {
    CaptureResult result{false, frequencyMhz, 0, 0, 0, ""};

    // Stub: En production utiliser CC1101 ou similaire
    // Pour ESP32 : utiliser esp_wifi_set_promiscuous en mode sub-GHz
    // ou module externe CC1101 (SPI)

    result.error = "Sub-GHz module not yet integrated. Awaiting CC1101 module.";
    return result;
}

ReplayResult replay(uint8_t repeatCount, uint16_t delayMs) {
    ReplayResult result{false, 0, 0, "", ""};

    // Stub: replay nécessite transmission sub-GHz
    result.error = "Replay requires Sub-GHz transmitter (not available without hardware)";
    return result;
}

String analyzePattern(uint16_t samples) {
    // Stub pattern analysis
    return "Pattern: Unknown (await real hardware)";
}

} // namespace SubGhzReplay
