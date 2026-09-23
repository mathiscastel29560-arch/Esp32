#pragma once
#include <Arduino.h>
#include <vector>

// PN532 NFC/RFID reader on I2C bus
// SDA = GPIO 8, SCL = GPIO 9
// IRQ = GPIO 47 (optional, for interrupt-driven reading)
namespace NfcTools {

struct TagInfo {
    uint8_t uid[10];
    uint8_t uidLen;
    String type;
    uint16_t capacity;
    bool isWritable;
};

struct ScanResult {
    bool found;
    TagInfo tag;
    uint32_t readTimeMs;
    String error;
};

// Scan for a Mifare or other NFC tag (blocking)
ScanResult scan(uint32_t timeoutMs = 5000);

// Read data from a sector of a Mifare Classic card
String readSector(uint8_t sector);

// Write data to a sector of a Mifare Classic card (requires auth)
bool writeSector(uint8_t sector, const uint8_t *data);

// Clone a tag to another blank tag (source must be detected)
bool cloneTag(const TagInfo &source, uint32_t timeoutMs = 30000);

} // namespace NfcTools
