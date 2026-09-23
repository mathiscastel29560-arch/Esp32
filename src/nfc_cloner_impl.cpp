#include "nfc_cloner.h"
#include <Wire.h>

#define PN532_I2C_ADDRESS 0x48

namespace NfcCloner {

class PN532Reader {
public:
    bool begin() {
        Wire.begin();
        Wire.setClock(100000);

        if (!isPN532Present()) {
            Serial.println("PN532 not found on I2C bus");
            return false;
        }

        return true;
    }

private:
    bool isPN532Present() {
        Wire.beginTransmission(PN532_I2C_ADDRESS);
        return Wire.endTransmission() == 0;
    }
};

static PN532Reader nfc;

ReadResult readNfcTag(uint32_t durationMs) {
    ReadResult result = {false, "", "", 0};

    uint32_t startTime = millis();

    if (!nfc.begin()) {
        result.tagUid = "ERROR";
        result.tagContent = "PN532 initialization failed";
        return result;
    }

    Serial.println("Waiting for NFC tag...");

    uint32_t deadline = startTime + durationMs;
    while ((int32_t)(millis() - deadline) < 0) {
        uint8_t cmd[] = {0x00, 0x00, 0xFF, 0x04, 0xFC, 0xD4, 0x4A, 0x01, 0x00, 0xE1, 0x00};

        Wire.beginTransmission(PN532_I2C_ADDRESS);
        Wire.write(cmd, sizeof(cmd));
        if (Wire.endTransmission() != 0) {
            delay(100);
            continue;
        }

        delay(100);

        Wire.requestFrom(PN532_I2C_ADDRESS, 32);

        uint8_t response[32];
        int len = 0;
        while (Wire.available() && len < 32) {
            response[len++] = Wire.read();
        }

        if (len > 10 && response[5] == 0x4B) {
            char uidBuf[15];
            snprintf(uidBuf, sizeof(uidBuf), "%02X%02X%02X%02X",
                     response[11], response[12], response[13], response[14]);

            result.tagUid = String(uidBuf);
            result.tagContent = "NDEF: Tag detected\n";
            result.tagContent += "Type: NTAG216\n";
            result.tagContent += "UID: " + result.tagUid;
            result.success = true;

            Serial.printf("Tag found: %s\n", uidBuf);
            break;
        }

        delay(100);
    }

    result.durationMs = millis() - startTime;
    return result;
}

CloneResult cloneNfcTag(const char* sourceUid, uint32_t durationMs) {
    CloneResult result = {false, "", "", 0};

    uint32_t startTime = millis();

    if (!nfc.begin()) {
        result.sourceUid = String(sourceUid);
        result.clonedUid = "ERROR";
        return result;
    }

    Serial.printf("Cloning NFC tag: %s\n", sourceUid);

    uint8_t srcLen = strlen(sourceUid);
    uint8_t cloneCmd[20] = {0x00, 0x00, 0xFF, 0x10, 0xF0, 0xD4, 0x40, 0x01};

    memcpy(&cloneCmd[8], sourceUid, (srcLen > 12) ? 12 : srcLen);

    Wire.beginTransmission(PN532_I2C_ADDRESS);
    Wire.write(cloneCmd, 20);
    Wire.endTransmission();

    delay(500);

    Wire.requestFrom(PN532_I2C_ADDRESS, 20);
    uint8_t response[20];
    int len = 0;
    while (Wire.available() && len < 20) {
        response[len++] = Wire.read();
    }

    result.success = (len > 5 && response[5] == 0x41);
    result.sourceUid = String(sourceUid);
    result.clonedUid = result.success ? String(sourceUid) : "FAILED";
    result.durationMs = millis() - startTime;

    Serial.printf("Clone result: %s\n", result.success ? "SUCCESS" : "FAILED");

    return result;
}

WriteResult writeNdefPayload(const char* tagUid, const char* maliciousPayload, uint32_t durationMs) {
    WriteResult result = {false, "", 0};

    uint32_t startTime = millis();

    if (!nfc.begin()) {
        result.payload = String(maliciousPayload);
        return result;
    }

    Serial.printf("Writing NDEF payload to tag: %s\n", tagUid);

    uint32_t deadline = startTime + durationMs;
    uint8_t offset = 0x04;

    while ((int32_t)(millis() - deadline) < 0 && offset < 0xFF) {
        uint8_t writeCmd[32] = {0x00, 0x00, 0xFF, 0x1A, 0xE6, 0xD4, 0x40, 0x02};
        writeCmd[8] = offset;

        uint8_t payloadLen = strlen(maliciousPayload);
        memcpy(&writeCmd[9], maliciousPayload, (payloadLen > 23) ? 23 : payloadLen);

        Wire.beginTransmission(PN532_I2C_ADDRESS);
        Wire.write(writeCmd, 32);
        if (Wire.endTransmission() == 0) {
            delay(100);
            result.success = true;
            break;
        }

        offset++;
        delay(50);
    }

    result.payload = String(maliciousPayload);
    result.durationMs = millis() - startTime;

    Serial.printf("Write result: %s\n", result.success ? "SUCCESS" : "FAILED");

    return result;
}

}  // namespace NfcCloner
