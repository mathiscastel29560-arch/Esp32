#pragma once
#include <Arduino.h>

namespace SmarthomeHijacker {

// Philips Hue Bridge hijacking
struct HueResult {
    bool success;
    uint32_t devicesControlled;
    uint32_t durationMs;
    String bridgeIp;
};
HueResult hijackPhilipsHue(const char* bridgeIp = "192.168.1.100", uint32_t durationMs = 20000);

// Nest/Ring enumeration and control
struct NestResult {
    bool success;
    uint32_t devicesFound;
    uint32_t durationMs;
    String actionPerformed;
};
NestResult enumerateNestDevices(uint32_t durationMs = 15000);

// IKEA Tradfri pairing attack
struct TradfriResult {
    bool success;
    uint32_t devicesJoined;
    uint32_t durationMs;
    String commandType;
};
TradfriResult tradfriPairingAttack(uint32_t durationMs = 25000);

// Amazon Alexa device discovery and control
struct AlexaResult {
    bool success;
    uint32_t devicesDiscovered;
    uint32_t commandsSent;
    uint32_t durationMs;
};
AlexaResult discoverAlexaDevices(uint32_t durationMs = 20000);

}  // namespace SmarthomeHijacker
