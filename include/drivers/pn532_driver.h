#pragma once
#include <Arduino.h>

namespace PN532Driver {

// PN532 Commands
#define PN532_COMMAND_GETFIRMWAREVERSION  0x03
#define PN532_COMMAND_SAMCONFIGURATION    0x14
#define PN532_COMMAND_RFCONFIGURATION     0x32
#define PN532_COMMAND_INLISTPASSIVETARGET 0x4A
#define PN532_COMMAND_INDATAEXCHANGE      0x40
#define PN532_COMMAND_MIFARECLASSICTARGET 0x51

// Card types
#define PN532_MIFARE_CLASSIC_1K   0x04
#define PN532_MIFARE_CLASSIC_4K   0x08

struct Card {
    uint8_t uid[10];
    uint8_t uidLen;
    uint8_t cardType;
    bool isNFC;
};

struct BlockData {
    uint8_t data[16];
};

// Initialize PN532 module (I2C)
bool init();

// Deinitialize
void deinit();

// Get firmware version
uint32_t getFirmwareVersion();

// Scan for NFC cards/tags
bool scanCard(Card& card);

// Read block from Mifare card (16 bytes)
bool readBlock(const Card& card, uint8_t blockNum, BlockData& data);

// Write block to Mifare card (16 bytes)
bool writeBlock(const Card& card, uint8_t blockNum, const BlockData& data);

// Authenticate block with key A
bool authenticateBlock(const Card& card, uint8_t blockNum, const uint8_t keyA[6]);

// Get card UID as string
String getUIDString(const Card& card);

}  // namespace PN532Driver
