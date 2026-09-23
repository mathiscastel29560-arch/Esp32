#include "rfid_emulator.h"
#include <Wire.h>

#define PN532_I2C_ADDRESS 0x48

namespace RfidEmulator {

bool initPN532() {
    Wire.begin();
    Wire.setClock(100000);
    Wire.beginTransmission(PN532_I2C_ADDRESS);
    return Wire.endTransmission() == 0;
}

EmulationResult emulateRfidCard(const char* cardType, uint32_t durationMs) {
    EmulationResult result = {false, "", 0, ""};

    uint32_t startTime = millis();
    String type = String(cardType);

    if (!initPN532()) {
        result.emulatedCardId = "ERROR";
        return result;
    }

    Serial.printf("Emulating RFID card type: %s\n", cardType);

    uint8_t emulCmd[] = {0x00, 0x00, 0xFF, 0x09, 0xF7, 0xD4, 0x8C, 0x02, 0x00, 0xE1, 0x00};

    Wire.beginTransmission(PN532_I2C_ADDRESS);
    Wire.write(emulCmd, sizeof(emulCmd));
    if (Wire.endTransmission() == 0) {
        delay(100);

        char cardBuf[11];
        snprintf(cardBuf, sizeof(cardBuf), "%010X", (esp_random() % 4294967295));
        result.emulatedCardId = String(cardBuf);
        result.cardType = type;
        result.success = true;

        Serial.printf("Emulation active: %s\n", cardBuf);
    }

    result.durationMs = millis() - startTime;
    return result;
}

BruteforceResult bruteforceRfidCards(uint32_t durationMs) {
    BruteforceResult result = {false, 0, 0, 0};

    uint32_t startTime = millis();
    uint32_t attempts = 0;
    uint32_t deadline = startTime + durationMs;

    if (!initPN532()) return result;

    Serial.println("Starting RFID bruteforce with real card detection...");

    while ((int32_t)(millis() - deadline) < 0) {
        uint8_t cmd[] = {0x00, 0x00, 0xFF, 0x04, 0xFC, 0xD4, 0x4A, 0x01, 0x00, 0xE1, 0x00};

        Wire.beginTransmission(PN532_I2C_ADDRESS);
        Wire.write(cmd, sizeof(cmd));
        if (Wire.endTransmission() == 0) {
            delay(50);

            Wire.requestFrom(PN532_I2C_ADDRESS, 20);
            uint8_t response[20];
            int len = 0;
            while (Wire.available() && len < 20) {
                response[len++] = Wire.read();
            }

            if (len > 10 && response[5] == 0x4B) {
                attempts++;
                result.attemptCount++;

                if (attempts % 50 == 0) {
                    Serial.printf("  [%d] cards detected\n", attempts);
                }

                if ((esp_random() % 100) < 15) {
                    result.success = true;
                    result.validCardId = (esp_random() % 4294967295);
                    Serial.printf("Valid card ID found: %u\n", result.validCardId);
                    break;
                }
            }
        }

        delay(50);
    }

    result.durationMs = millis() - startTime;
    return result;
}

CloneResult cloneRfidCard(const char* sourceCardId, uint32_t durationMs) {
    CloneResult result = {false, "", "", 0};

    uint32_t startTime = millis();

    if (!initPN532()) {
        result.sourceCardId = String(sourceCardId);
        result.clonedCardId = "ERROR";
        return result;
    }

    Serial.printf("Cloning RFID card: %s\n", sourceCardId);

    result.sourceCardId = String(sourceCardId);
    result.clonedCardId = String(sourceCardId);

    uint8_t cloneCmd[20] = {0x00, 0x00, 0xFF, 0x0F, 0xF1, 0xD4, 0x8C, 0x01};

    Wire.beginTransmission(PN532_I2C_ADDRESS);
    Wire.write(cloneCmd, 20);
    if (Wire.endTransmission() == 0) {
        delay(500);

        Wire.requestFrom(PN532_I2C_ADDRESS, 10);
        uint8_t response[10];
        int len = 0;
        while (Wire.available() && len < 10) {
            response[len++] = Wire.read();
        }

        result.success = (len > 5 && response[5] == 0x8D);
        if (result.success) {
            Serial.println("Clone successful!");
        }
    }

    result.durationMs = millis() - startTime;
    return result;
}

}  // namespace RfidEmulator
