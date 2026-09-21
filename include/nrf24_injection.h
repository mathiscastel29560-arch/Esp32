#pragma once
#include <Arduino.h>
#include <vector>

namespace Nrf24Injection {

struct InjectionResult {
    bool success;
    uint32_t packetsInjected;
    uint8_t channel;
};

// Inject custom packets into NRF24 network
InjectionResult injectPacket(uint8_t channel, const std::vector<uint8_t> &payload, uint8_t repeatCount = 3);

}  // namespace Nrf24Injection
