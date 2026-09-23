#include "mifare_classic.h"
#include <Wire.h>

#define PN532_I2C_ADDRESS 0x48

namespace MifareClassic {

bool initPN532() {
    Wire.begin();
    Wire.setClock(100000);
    Wire.beginTransmission(PN532_I2C_ADDRESS);
    return Wire.endTransmission() == 0;
}

ReadResult readMifareCard(uint32_t durationMs) {
    ReadResult result = {false, "", 0};

    uint32_t startTime = millis();

    if (!initPN532()) {
        result.sectorData = "ERROR: PN532 not detected";
        return result;
    }

    Serial.println("Reading Mifare Classic sectors via PN532...");

    String data = "";

    for (uint8_t sector = 0; sector < 4; sector++) {
        uint8_t readCmd[32] = {0x00, 0x00, 0xFF, 0x1A, 0xE6, 0xD4, 0x40, 0x01, sector};
        memset(&readCmd[9], 0xFF, 6);

        Wire.beginTransmission(PN532_I2C_ADDRESS);
        Wire.write(readCmd, 32);
        if (Wire.endTransmission() == 0) {
            delay(100);

            Wire.requestFrom(PN532_I2C_ADDRESS, 32);
            uint8_t response[32];
            int len = 0;
            while (Wire.available() && len < 32) {
                response[len++] = Wire.read();
            }

            if (len > 16) {
                char sectorLine[64];
                snprintf(sectorLine, sizeof(sectorLine), "Sector_%d: ", sector);
                for (int i = 0; i < 16 && i < len - 5; i++) {
                    snprintf(&sectorLine[strlen(sectorLine)], 4, "%02X", response[i + 5]);
                }
                data += String(sectorLine) + "\n";
            }
        }
    }

    result.sectorData = data;
    result.success = true;
    result.durationMs = millis() - startTime;

    Serial.println("Mifare read complete");
    return result;
}

KeyRecoveryResult recoverMifareKeys(uint32_t durationMs) {
    KeyRecoveryResult result = {false, "", 0, 0};

    uint32_t startTime = millis();
    uint32_t attempts = 0;
    uint32_t deadline = startTime + durationMs;

    if (!initPN532()) return result;

    const char* defaultKeys[] = {
        "FFFFFFFFFFFF", "000000000000", "A0A1A2A3A4A5", "D3F7D3F7D3F7", "058076F66FFF"
    };

    Serial.println("Attempting Mifare key recovery with PN532...");

    while ((int32_t)(millis() - deadline) < 0) {
        for (int i = 0; i < 5; i++) {
            uint8_t keyCmd[32] = {0x00, 0x00, 0xFF, 0x16, 0xEA, 0xD4, 0x40, 0x01, 0x00};

            const char* key = defaultKeys[i];
            for (int j = 0; j < 6; j++) {
                sscanf(&key[j*2], "%2hhx", &keyCmd[9 + j]);
            }

            Wire.beginTransmission(PN532_I2C_ADDRESS);
            Wire.write(keyCmd, 32);
            if (Wire.endTransmission() == 0) {
                delay(50);

                Wire.requestFrom(PN532_I2C_ADDRESS, 10);
                uint8_t response[10];
                int len = 0;
                while (Wire.available() && len < 10) {
                    response[len++] = Wire.read();
                }

                attempts++;

                if (len > 5 && response[5] == 0x41) {
                    result.success = true;
                    result.keyFound = defaultKeys[i];
                    Serial.printf("Key found: %s\n", defaultKeys[i]);
                    break;
                }
            }
        }

        if (result.success) break;
        delay(100);
    }

    result.attemptCount = attempts;
    result.durationMs = millis() - startTime;

    return result;
}

CloneResult cloneMifareCard(const char* sourceUid, uint32_t durationMs) {
    CloneResult result = {false, "", "", 0};

    uint32_t startTime = millis();

    if (!initPN532()) {
        result.sourceUid = String(sourceUid);
        result.clonedUid = "ERROR";
        return result;
    }

    Serial.printf("Cloning Mifare card: %s\n", sourceUid);

    result.sourceUid = String(sourceUid);
    result.clonedUid = String(sourceUid);

    uint8_t cloneCmd[20] = {0x00, 0x00, 0xFF, 0x10, 0xF0, 0xD4, 0x40, 0x02};
    memcpy(&cloneCmd[8], sourceUid, 10);

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

        result.success = (len > 5 && response[5] == 0x41);
        if (result.success) {
            Serial.println("Mifare clone successful!");
        }
    }

    result.durationMs = millis() - startTime;
    return result;
}

}  // namespace MifareClassic
