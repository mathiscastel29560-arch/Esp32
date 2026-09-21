#include "ir_bruteforce.h"
#include "config.h"
#include <IRsend.h>

namespace {
IRsend irsend(PIN_IR_TX);

// TV Power codes - Most common protocols (NEC, Sony, RC5)
struct IrCode {
    const char *name;
    uint64_t code;
    uint16_t bits;
    decode_type_t protocol;  // NEC=0, SONY=1, RC5=2, etc
};

// Common TV power codes across protocols
const IrCode TV_POWER_CODES[] = {
    // NEC Protocol (32-bit codes) - Most common
    {"Samsung TV", 0xE0E040BF, 32, NEC},
    {"LG TV", 0x20DF10EF, 32, NEC},
    {"Sony TV", 0xA90, 12, SONY},
    {"Philips TV", 0x0C, 12, RC5},
    {"Panasonic TV", 0x100BCBD, 24, NEC},
    {"Toshiba TV", 0x40E0E01F, 32, NEC},
    {"Sharp TV", 0x40E10D0F, 32, NEC},
    {"Hitachi TV", 0x00FF807F, 32, NEC},
    {"JVC TV", 0x4CB34B54, 32, NEC},
    {"Pioneer TV", 0xA45A05FA, 32, NEC},
    // Additional Samsung variants
    {"Samsung Alt1", 0xE0E08877, 32, NEC},
    {"Samsung Alt2", 0xE0E020DF, 32, NEC},
    // Additional LG variants
    {"LG Alt1", 0x20DF00FF, 32, NEC},
    {"LG Alt2", 0x20DFC03F, 32, NEC},
    // RC6 Protocol
    {"Philips RC6", 0x10, 16, RC6},
};

const size_t TV_POWER_COUNT = sizeof(TV_POWER_CODES) / sizeof(TV_POWER_CODES[0]);

// AC / Climate control codes
const IrCode AC_CODES[] = {
    {"Daikin AC", 0x11E1402F, 32, NEC},
    {"Fujitsu AC", 0x40A13CE4, 32, NEC},
    {"LG AC", 0x88C0051F, 32, NEC},
    {"Midea AC", 0x20FE04FB, 32, NEC},
};

const size_t AC_CODES_COUNT = sizeof(AC_CODES) / sizeof(AC_CODES[0]);

// Light/Lamp codes
const IrCode LIGHT_CODES[] = {
    {"Generic Light On", 0xFFA25D, 24, NEC},
    {"Generic Light Off", 0xFF629D, 24, NEC},
    {"Philips Hue", 0x100BCBD, 24, NEC},
    {"LIFX Light", 0x40E0E01F, 32, NEC},
};

const size_t LIGHT_CODES_COUNT = sizeof(LIGHT_CODES) / sizeof(LIGHT_CODES[0]);
}

namespace IRBruteforce {

std::vector<String> getCommonCodes(const String &deviceType) {
    std::vector<String> codes;

    if (deviceType == "TV") {
        for (size_t i = 0; i < TV_POWER_COUNT; i++) {
            codes.push_back(TV_POWER_CODES[i].name);
        }
    } else if (deviceType == "AC") {
        for (size_t i = 0; i < AC_CODES_COUNT; i++) {
            codes.push_back(AC_CODES[i].name);
        }
    } else if (deviceType == "LIGHT") {
        for (size_t i = 0; i < LIGHT_CODES_COUNT; i++) {
            codes.push_back(LIGHT_CODES[i].name);
        }
    } else {
        // Default: TV power codes
        for (size_t i = 0; i < TV_POWER_COUNT; i++) {
            codes.push_back(TV_POWER_CODES[i].name);
        }
    }

    return codes;
}

BruteResult bruteForce(const String &deviceType, uint16_t timeoutMs) {
    BruteResult result{false, "", 0, deviceType};

    Serial.println("\n=== IR Bruteforce Started ===");
    Serial.println("Device: " + deviceType);
    Serial.println("Timeout: " + String(timeoutMs) + "ms");
    Serial.println("Delay: 300ms between codes\n");

    const IrCode *codeArray = nullptr;
    size_t codeCount = 0;

    if (deviceType == "TV") {
        codeArray = TV_POWER_CODES;
        codeCount = TV_POWER_COUNT;
    } else if (deviceType == "AC") {
        codeArray = AC_CODES;
        codeCount = AC_CODES_COUNT;
    } else if (deviceType == "LIGHT") {
        codeArray = LIGHT_CODES;
        codeCount = LIGHT_CODES_COUNT;
    } else {
        codeArray = TV_POWER_CODES;
        codeCount = TV_POWER_COUNT;
    }

    unsigned long startTime = millis();
    uint32_t attemptsCount = 0;

    for (size_t i = 0; i < codeCount; i++) {
        if (millis() - startTime > timeoutMs) {
            Serial.println("Timeout reached!");
            break;
        }

        attemptsCount++;
        Serial.print("  [" + String(attemptsCount) + "/" + String(codeCount) + "] ");
        Serial.println(codeArray[i].name);

        // Send IR code using IRremoteESP8266
        irsend.send(codeArray[i].protocol, codeArray[i].code, codeArray[i].bits);
        delay(300);  // Wait 300ms between attempts
    }

    result.attemptsCount = attemptsCount;
    Serial.println("\n=== Bruteforce Complete ===");
    Serial.println("Attempted " + String(attemptsCount) + " codes");
    Serial.println("Check if target device responded!");

    return result;
}

} // namespace IRBruteforce
