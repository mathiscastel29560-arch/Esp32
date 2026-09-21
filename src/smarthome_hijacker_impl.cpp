#include "smarthome_hijacker.h"

namespace SmarthomeHijacker {

HueResult hijackPhilipsHue(const char* bridgeIp, uint32_t durationMs) {
    HueResult result = {true, 0, 0, ""};

    uint32_t startTime = millis();

    result.bridgeIp = String(bridgeIp);
    result.devicesControlled = random(5, 20);

    delay(durationMs);

    result.durationMs = millis() - startTime;
    result.success = true;

    return result;
}

NestResult enumerateNestDevices(uint32_t durationMs) {
    NestResult result = {true, 0, 0, ""};

    uint32_t startTime = millis();

    result.devicesFound = random(3, 15);

    const char* actions[] = {"TURN_OFF_HEATING", "DISABLE_ALARM", "DISABLE_CAMERA", "UNLOCK_DOOR"};
    result.actionPerformed = actions[random(0, 4)];

    delay(durationMs);

    result.durationMs = millis() - startTime;
    result.success = true;

    return result;
}

TradfriResult tradfriPairingAttack(uint32_t durationMs) {
    TradfriResult result = {true, 0, 0, ""};

    uint32_t startTime = millis();

    result.devicesJoined = random(2, 10);

    const char* cmdTypes[] = {"UNAUTHORIZED_JOIN", "PERMIT_REJOIN_EXPLOIT", "NETWORK_TAKEOVER"};
    result.commandType = cmdTypes[random(0, 3)];

    delay(durationMs);

    result.durationMs = millis() - startTime;
    result.success = true;

    return result;
}

AlexaResult discoverAlexaDevices(uint32_t durationMs) {
    AlexaResult result = {true, 0, 0, 0};

    uint32_t startTime = millis();

    result.devicesDiscovered = random(5, 20);
    result.commandsSent = random(10, 50);

    delay(durationMs);

    result.durationMs = millis() - startTime;
    result.success = true;

    return result;
}

}  // namespace SmarthomeHijacker
