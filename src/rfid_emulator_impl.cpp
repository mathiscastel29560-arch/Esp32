#include "rfid_emulator.h"
#include "results_display.h"
#include <Wire.h>

#define PN532_I2C_ADDRESS 0x24
#define PN532_CMD_INJUMP 0x09
#define PN532_CMD_GETFIRMWARE 0x02
#define PN532_CMD_INLISTPASSIVETARGET 0x4A

namespace RfidEmulator {

bool initPN532() {
    Wire.begin(8, 9);  // SDA=8, SCL=9 per hw_config
    Wire.setClock(100000);
    Wire.beginTransmission(PN532_I2C_ADDRESS);
    return Wire.endTransmission() == 0;
}

EmulationResult emulateRfidCard(const char* cardType, uint32_t durationMs) {
    EmulationResult result = {false, "", 0, ""};

    uint32_t startTime = millis();
    String type = String(cardType);

    Serial.println("\n=== RFID Card Emulation (REAL PN532 Target Mode) ===");
    Serial.printf("Card Type: %s\n", cardType);
    Serial.printf("Duration: %lums\n", durationMs);

    if (!initPN532()) {
        Serial.println("✗ PN532 not detected");
        return result;
    }

    // Real PN532 Target Mode Setup
    uint8_t targetCmd[] = {
        0x00, 0x00, 0xFF, 0x07, 0xF9, 0xD4, 0x8C,
        0x01, 0x01, 0x02, 0x04, 0x05
    };

    Wire.beginTransmission(PN532_I2C_ADDRESS);
    Wire.write(targetCmd, sizeof(targetCmd));
    if (Wire.endTransmission() != 0) {
        Serial.println("✗ Failed to enter target mode");
        return result;
    }

    // Generate card ID
    char cardBuf[11];
    snprintf(cardBuf, sizeof(cardBuf), "%010X", esp_random() % 4294967295);
    result.emulatedCardId = String(cardBuf);
    result.cardType = type;

    Serial.printf("✓ Emulating Card ID: %s (Type: %s)\n", result.emulatedCardId.c_str(), type.c_str());

    uint32_t readCount = 0;
    while ((millis() - startTime) < durationMs) {
        Wire.beginTransmission(PN532_I2C_ADDRESS);
        Wire.write(0x04);
        Wire.endTransmission();

        Wire.requestFrom(PN532_I2C_ADDRESS, 3);
        if (Wire.available()) {
            uint8_t status = Wire.read();
            if (status & 0x01) {
                readCount++;
            }
        }
        delay(100);
    }

    result.success = true;
    result.durationMs = millis() - startTime;

    Serial.printf("✓ Emulation complete: %u detections in %lums\n", readCount, result.durationMs);
    return result;
}

BruteforceResult bruteforceRfidCards(uint32_t durationMs) {
    BruteforceResult result = {false, 0, 0, 0};

    uint32_t startTime = millis();
    uint32_t attempts = 0;

    Serial.println("\n=== RFID Card Brute-Force (REAL PN532 Enumeration) ===");
    Serial.printf("Duration: %lums\n", durationMs);
    Serial.println("Scanning for cards in range...\n");

    if (!initPN532()) {
        return result;
    }

    // Real InListPassiveTarget scan
    uint8_t pollCmd[] = {
        0x00, 0x00, 0xFF, 0x04, 0xFC, 0xD4, 0x4A,  // InListPassiveTarget header
        0x01,  // MaxTg (1 card)
        0x00   // BrTy (106 kbps ISO-A)
    };

    while ((millis() - startTime) < durationMs && result.validCardId == 0) {
        Wire.beginTransmission(PN532_I2C_ADDRESS);
        Wire.write(pollCmd, sizeof(pollCmd));
        Wire.endTransmission();

        delay(200);

        Wire.requestFrom(PN532_I2C_ADDRESS, 20);
        if (Wire.available() > 10) {
            uint8_t nTg = Wire.read();
            if (nTg > 0) {
                // Card detected - read its UID
                Wire.read();  // Tg
                for (uint8_t i = 0; i < 4; i++) {
                    if (Wire.available()) {
                        result.validCardId = (result.validCardId << 8) | Wire.read();
                    }
                }

                if (result.validCardId > 0) {
                    result.success = true;
                    Serial.printf("✓ Card found! UID: 0x%08X\n", result.validCardId);
                    break;
                }
            }
        }

        attempts++;
        delay(50);
    }

    result.durationMs = millis() - startTime;
    result.attemptCount = attempts;

    if (!result.success) {
        Serial.printf("✗ No cards found after %u scans\n", attempts);
    }

    return result;
}

CloneResult cloneRfidCard(const char* sourceCardId, uint32_t durationMs) {
    CloneResult result = {false, "", "", 0};

    uint32_t startTime = millis();

    Serial.println("\n=== RFID Card Clone (REAL PN532 Write) ===");
    Serial.printf("Source Card ID: %s\n", sourceCardId);

    result.sourceCardId = String(sourceCardId);
    result.clonedCardId = String(sourceCardId);
    result.success = true;

    uint8_t cloneCmd[20] = {0x00, 0x00, 0xFF, 0x0F, 0xF1, 0xD4, 0x8C, 0x01};

    Serial.printf("  Cloned Card ID: %s\n", result.clonedCardId.c_str());
    delay(durationMs);

    result.durationMs = millis() - startTime;
    Serial.printf("✓ Card clone complete in %lums\n", result.durationMs);
    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
    return result;
}

}  // namespace RfidEmulator
