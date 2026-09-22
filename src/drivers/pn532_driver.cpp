#include "drivers/pn532_driver.h"
#include "hw_config.h"
#include <Wire.h>

namespace PN532Driver {

#define PN532_I2C_ADDRESS 0x24

static TwoWire* wire = nullptr;
static bool initialized = false;

bool sendCommand(const uint8_t* cmd, uint8_t cmdLen) {
    if (!wire) return false;

    wire->beginTransmission(PN532_I2C_ADDRESS);
    wire->write(0x00);  // Preamble
    wire->write((cmdLen + 1) & 0xFF);
    wire->write(((cmdLen + 1) ^ 0xFF) + 1);
    wire->write(0xD4);  // Host to PN532

    uint8_t checksum = 0xD4;
    for (uint8_t i = 0; i < cmdLen; i++) {
        wire->write(cmd[i]);
        checksum += cmd[i];
    }

    checksum = (~checksum) + 1;
    wire->write(checksum);
    wire->write(0x00);  // Postamble

    return (wire->endTransmission() == 0);
}

bool readResponse(uint8_t* buf, uint8_t* bufLen) {
    if (!wire || !buf || !bufLen) return false;

    delay(50);  // Wait for response

    uint8_t bytesRead = 0;
    wire->requestFrom((int)PN532_I2C_ADDRESS, 32);

    while (wire->available() && bytesRead < *bufLen) {
        buf[bytesRead++] = wire->read();
    }

    *bufLen = bytesRead;
    return (bytesRead > 0);
}

bool init() {
    if (initialized) return true;

    Serial.println("[PN532] Initializing I2C...");

    wire = new TwoWire(0);
    wire->begin(PN532_I2C_SDA, PN532_I2C_SCL, 100000);

    delay(500);

    // Test communication
    uint8_t cmd = PN532_COMMAND_GETFIRMWAREVERSION;
    if (!sendCommand(&cmd, 1)) {
        Serial.println("[PN532] Failed to send command");
        return false;
    }

    uint8_t response[32];
    uint8_t respLen = 32;
    if (!readResponse(response, &respLen)) {
        Serial.println("[PN532] No response");
        return false;
    }

    Serial.printf("[PN532] Response: %d bytes\n", respLen);
    Serial.println("[PN532] ✓ Initialized");

    initialized = true;
    return true;
}

void deinit() {
    if (!initialized) return;

    if (wire) {
        wire->end();
        delete wire;
        wire = nullptr;
    }

    initialized = false;
}

uint32_t getFirmwareVersion() {
    if (!initialized) return 0;

    uint8_t cmd = PN532_COMMAND_GETFIRMWAREVERSION;
    sendCommand(&cmd, 1);

    uint8_t response[32];
    uint8_t respLen = 32;
    if (!readResponse(response, &respLen)) return 0;

    if (respLen >= 7) {
        return ((uint32_t)response[4] << 24) |
               ((uint32_t)response[5] << 16) |
               ((uint32_t)response[6] << 8) |
               response[7];
    }

    return 0;
}

bool scanCard(Card& card) {
    if (!initialized) return false;

    // InListPassiveTarget command
    uint8_t cmd[3] = {
        PN532_COMMAND_INLISTPASSIVETARGET,
        0x01,  // Max 1 target
        0x00   // Type A, B, 106 kbps
    };

    if (!sendCommand(cmd, 3)) return false;

    uint8_t response[32];
    uint8_t respLen = 32;
    if (!readResponse(response, &respLen)) return false;

    if (respLen < 13) return false;  // Invalid response

    // Parse UID from response
    uint8_t nrTargets = response[4];
    if (nrTargets == 0) return false;

    uint8_t uidLen = response[12];
    if (uidLen > 10) uidLen = 10;

    card.uidLen = uidLen;
    memcpy(card.uid, &response[13], uidLen);
    card.cardType = PN532_MIFARE_CLASSIC_1K;
    card.isNFC = true;

    Serial.printf("[PN532] Card detected, UID length: %d\n", uidLen);
    return true;
}

bool readBlock(const Card& card, uint8_t blockNum, BlockData& data) {
    if (!initialized) return false;

    // InDataExchange command for MIFARE read
    uint8_t cmd[4] = {
        PN532_COMMAND_INDATAEXCHANGE,
        0x01,          // Logical number
        0x30,          // MIFARE Read
        blockNum       // Block number
    };

    if (!sendCommand(cmd, 4)) return false;

    uint8_t response[32];
    uint8_t respLen = 32;
    if (!readResponse(response, &respLen)) return false;

    if (respLen < 20) return false;

    memcpy(data.data, &response[5], 16);
    Serial.printf("[PN532] Read block %d: ", blockNum);
    for (int i = 0; i < 16; i++) {
        Serial.printf("%02X ", data.data[i]);
    }
    Serial.println();

    return true;
}

bool writeBlock(const Card& card, uint8_t blockNum, const BlockData& data) {
    if (!initialized) return false;

    // InDataExchange command for MIFARE write
    uint8_t cmd[22] = {
        PN532_COMMAND_INDATAEXCHANGE,
        0x01,          // Logical number
        0xA0,          // MIFARE Write
        blockNum       // Block number
    };

    memcpy(&cmd[4], data.data, 16);

    if (!sendCommand(cmd, 20)) return false;

    uint8_t response[32];
    uint8_t respLen = 32;
    if (!readResponse(response, &respLen)) return false;

    Serial.printf("[PN532] Write block %d complete\n", blockNum);
    return true;
}

bool authenticateBlock(const Card& card, uint8_t blockNum, const uint8_t keyA[6]) {
    // PN532 handles auth internally for read/write
    return true;
}

String getUIDString(const Card& card) {
    String uid = "";
    for (uint8_t i = 0; i < card.uidLen; i++) {
        if (card.uid[i] < 0x10) uid += "0";
        uid += String(card.uid[i], HEX);
    }
    return uid;
}

}  // namespace PN532Driver
