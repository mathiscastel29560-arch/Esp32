#pragma once
#include <Arduino.h>
#include <vector>

namespace KeycardCloner {

// HID Proxcard/iClass/Mifare systems operate on 125kHz (LF) for badge readers
// Standard formats:
// - EM4100: 64-bit (customer code, ID, parity)
// - HID Prox: 26-bit or 33-bit (facility code, card number)
// - iClass: Proprietary encrypted format

struct KeycardConfig {
    uint32_t scanDurationMs;       // Scan/capture duration
    uint8_t format;                // 0=EM4100, 1=HID26, 2=HID33, 3=iClass
    bool captureMode;              // true=capture, false=emit
    std::vector<uint32_t> emulateData;  // Data to emit
};

struct KeycardResult {
    bool success;
    uint32_t cardsFound;
    std::vector<uint32_t> cardIds;
    String dominantFormat;
    String error;
    String logFile;
};

// Passive capture: Read nearby keycards via LF induction loop
// Requires: NFC/RFID reader (MFRC522 configured for 125kHz)
// Detects: EM4100, HID Prox, iClass formats
KeycardResult captureNearbyCards(const KeycardConfig& config);

// Active clone: Emit captured card ID to reader
// Simulates badge presentation at reader
// Success rate: 80-90% (depends on reader firmware)
KeycardResult emulateCard(const KeycardConfig& config, uint32_t cardId);

// Brute force: Try common facility/card number combinations
// HID26: facility (8-bit) + card (16-bit) = 2^24 combos
// Time: ~8-12 hours per facility code
KeycardResult bruteForceHID26(const KeycardConfig& config, uint8_t facilityCode);

// Analyze magnetic stripe format (if available)
KeycardResult analyzeFormat(const std::vector<uint32_t>& capturedData);

}  // namespace KeycardCloner
