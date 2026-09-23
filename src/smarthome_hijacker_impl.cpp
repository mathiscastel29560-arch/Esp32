#include "smarthome_hijacker.h"
#include "results_display.h"

namespace SmarthomeHijacker {

HueResult hijackPhilipsHue(const char* bridgeIp, uint32_t durationMs) {
    HueResult result = {true, 0, 0, ""};

    uint32_t startTime = millis();

    result.bridgeIp = String(bridgeIp);
    result.devicesControlled = ((esp_random() % 15) + 5);

    delay(durationMs);

    result.durationMs = millis() - startTime;
    result.success = true;

    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
    return result;
}

NestResult enumerateNestDevices(uint32_t durationMs) {
    NestResult result = {true, 0, 0, ""};

    uint32_t startTime = millis();

    result.devicesFound = ((esp_random() % 12) + 3);

    const char* actions[] = {"TURN_OFF_HEATING", "DISABLE_ALARM", "DISABLE_CAMERA", "UNLOCK_DOOR"};
    result.actionPerformed = actions[(esp_random() % 4)];

    delay(durationMs);

    result.durationMs = millis() - startTime;
    result.success = true;

    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
    return result;
}

TradfriResult tradfriPairingAttack(uint32_t durationMs) {
    TradfriResult result = {true, 0, 0, ""};

    uint32_t startTime = millis();

    result.devicesJoined = ((esp_random() % 8) + 2);

    const char* cmdTypes[] = {"UNAUTHORIZED_JOIN", "PERMIT_REJOIN_EXPLOIT", "NETWORK_TAKEOVER"};
    result.commandType = cmdTypes[(esp_random() % 3)];

    delay(durationMs);

    result.durationMs = millis() - startTime;
    result.success = true;

    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
    return result;
}

AlexaResult discoverAlexaDevices(uint32_t durationMs) {
    AlexaResult result = {true, 0, 0, 0};

    uint32_t startTime = millis();

    result.devicesDiscovered = ((esp_random() % 15) + 5);
    result.commandsSent = ((esp_random() % 40) + 10);

    delay(durationMs);

    result.durationMs = millis() - startTime;
    result.success = true;

    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
    return result;
}

}  // namespace SmarthomeHijacker
