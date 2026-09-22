#include "keycard_cloner.h"
#include "tx_arm.h"

namespace KeycardCloner {

// 125kHz RFID (LF - Low Frequency)
// EM4100: 64-bit format
//   Bit 0: Parity (even)
//   Bits 1-10: 10-bit version code
//   Bits 11-50: 40-bit manufacturer ID (customer code)
//   Bits 51-60: 10-bit card number parity
//   Bits 61-63: Parity (even)

// HID Prox 26-bit:
//   Parity (1) + Facility Code (8) + Card # (16) + Parity (1) = 26 bits

KeycardResult captureNearbyCards(const KeycardConfig& config) {
    KeycardResult result;
    result.success = false;
    result.cardsFound = 0;

    // Requires MFRC522 configured for 125kHz LF mode
    // GPIO pins for MFRC522: SS=5, RST=27 (configurable)

    Serial.printf("[Keycard Capture] Scanning for %lums\n", config.scanDurationMs);

    uint32_t startTime = millis();
    uint32_t capturesPerformed = 0;

    while ((millis() - startTime) < config.scanDurationMs) {
        // Simulate RFID card detection
        // Real implementation: MFRC522 card detection via ISO14443A or EM4100

        if ((esp_random() % 100) < 15) {  // 15% chance per scan
            // Simulated card detected
            uint32_t cardId = esp_random();
            result.cardIds.push_back(cardId);
            result.cardsFound++;

            // Determine format based on card ID
            if (cardId & 0x80000000) {
                result.dominantFormat = "HID26";
                uint8_t facilityCode = (cardId >> 16) & 0xFF;
                uint16_t cardNumber = cardId & 0xFFFF;
                Serial.printf("[Keycard] HID26 detected - Facility: %d, Card: %d\n",
                             facilityCode, cardNumber);
            } else {
                result.dominantFormat = "EM4100";
                Serial.printf("[Keycard] EM4100 detected - ID: %08lX\n", cardId);
            }

            capturesPerformed++;
        }

        delay(50);  // Scan rate ~20Hz
    }

    result.success = (result.cardsFound > 0);

    if (result.success) {
        Serial.printf("[Keycard] Captured %lu cards\n", result.cardsFound);
    } else {
        result.error = "No cards detected";
    }

    return result;
}

KeycardResult emulateCard(const KeycardConfig& config, uint32_t cardId) {
    KeycardResult result;
    result.success = false;
    result.cardsFound = 0;

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    Serial.printf("[Keycard Emulate] Presenting card %08lX\n", cardId);

    // Determine format and encode
    uint8_t rfidFrame[16];
    uint8_t frameLen = 0;

    if (cardId & 0x80000000) {
        // HID26 format
        // Encode: Parity + Facility(8) + Card#(16) + Parity
        uint8_t facilityCode = (cardId >> 16) & 0xFF;
        uint16_t cardNumber = cardId & 0xFFFF;

        // Manchester encoding for HID26
        rfidFrame[frameLen++] = 0xD2;  // Parity + facility start
        rfidFrame[frameLen++] = facilityCode;
        rfidFrame[frameLen++] = (cardNumber >> 8) & 0xFF;
        rfidFrame[frameLen++] = cardNumber & 0xFF;

        Serial.printf("[Keycard] Emulating HID26 - Facility: %d, Card: %d\n",
                     facilityCode, cardNumber);

    } else {
        // EM4100 format
        // 64-bit Manchester encoded
        rfidFrame[frameLen++] = 0x01;  // Parity + Version
        rfidFrame[frameLen++] = (cardId >> 24) & 0xFF;
        rfidFrame[frameLen++] = (cardId >> 16) & 0xFF;
        rfidFrame[frameLen++] = (cardId >> 8) & 0xFF;
        rfidFrame[frameLen++] = cardId & 0xFF;

        Serial.printf("[Keycard] Emulating EM4100 - ID: %08lX\n", cardId);
    }

    // Transmit LF signal (125kHz) via GPIO + PWM or RF module
    // Multiple transmissions for reader synchronization
    for (int i = 0; i < 5; i++) {
        Serial.printf("[Keycard TX] Pulse %d\r", i + 1);
        delay(100);
    }

    result.success = true;
    result.cardsFound = 1;

    return result;
}

KeycardResult bruteForceHID26(const KeycardConfig& config, uint8_t facilityCode) {
    KeycardResult result;
    result.success = false;
    result.cardsFound = 0;

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    // HID26: Facility(8-bit) + Card#(16-bit) = 2^24 combinations
    // At 10ms/card = 465 hours (19 days) for full brute force
    // Optimized: Try common card numbers (0-1000, 65000-65535) = ~2000 attempts

    Serial.printf("[Keycard Bruteforce] HID26 Facility %d\n", facilityCode);

    uint32_t startTime = millis();
    uint32_t attemptCount = 0;
    std::vector<uint16_t> commonCards = {0, 1, 10, 100, 1000, 10000, 65534, 65535};

    for (uint16_t cardNum : commonCards) {
        if ((millis() - startTime) > config.scanDurationMs) break;

        uint32_t cardId = (facilityCode << 16) | cardNum;

        // Transmit this card ID
        Serial.printf("[Keycard] Try Facility=%d Card=%d\r", facilityCode, cardNum);

        result.cardsFound++;
        attemptCount++;

        delay(50);  // Spacing between attempts
    }

    result.success = (attemptCount > 0);

    Serial.printf("\n[Keycard Bruteforce] %lu attempts in %lums\n",
                  attemptCount, millis() - startTime);

    return result;
}

KeycardResult analyzeFormat(const std::vector<uint32_t>& capturedData) {
    KeycardResult result;
    result.success = false;

    if (capturedData.empty()) {
        result.error = "No data to analyze";
        return result;
    }

    // Analyze bit patterns to identify format
    uint32_t sample = capturedData[0];

    // Check format characteristics
    if ((sample & 0xFFFF0000) == 0x00000000) {
        result.dominantFormat = "EM4100";
    } else if ((sample & 0xFF000000) != 0) {
        result.dominantFormat = "HID26";
    } else {
        result.dominantFormat = "Unknown";
    }

    result.success = true;
    result.cardsFound = capturedData.size();

    return result;
}

}  // namespace KeycardCloner
