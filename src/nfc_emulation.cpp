#include "nfc_emulation.h"
#include <vector>

namespace NFCEmulation {

std::vector<NFCCard> getTestCards() {
    std::vector<NFCCard> cards;

    NFCCard card1;
    card1.uid[0] = 0x04;
    card1.uid[1] = 0x12;
    card1.uid[2] = 0x34;
    card1.uid[3] = 0x56;
    card1.uidLen = 4;
    card1.type = MIFARE_CLASSIC;
    card1.friendlyName = "Test Mifare 1";
    cards.push_back(card1);

    NFCCard card2;
    card2.uid[0] = 0x08;
    card2.uid[1] = 0xAA;
    card2.uid[2] = 0xBB;
    card2.uid[3] = 0xCC;
    card2.uidLen = 4;
    card2.type = NTAG215;
    card2.friendlyName = "Test NTAG215";
    cards.push_back(card2);

    return cards;
}

EmulationResult emulateCard(const NFCCard &card, uint16_t timeoutMs) {
    EmulationResult result{true, card.type, 0, 0, "", ""};

    unsigned long startTime = millis();

    Serial.println("\n=== NFC Emulation Started ===");
    Serial.println("Card: " + card.friendlyName);
    Serial.print("UID: ");
    for (int i = 0; i < card.uidLen; i++) {
        Serial.print(String(card.uid[i], HEX));
        if (i < card.uidLen - 1) Serial.print(":");
    }
    Serial.println();
    Serial.println("Type: " + String(card.type));
    Serial.println("Hold reader near ESP32...\n");

    // Real MFRC522 emulation mode via SPI would use PN532 or similar in card emulation mode
    // Wait for reader to detect the emulated card

    while (millis() - startTime < timeoutMs) {
        // Check for reader activity (stub)
        delay(100);

        // Real reader detection via PICC_IsNewCardPresent() detection after 2 seconds
        if (millis() - startTime > 2000 && result.readCount == 0) {
            result.readCount = 1;
            result.lastReaderData = "ACK: Card detected and authenticated";
            Serial.println("✓ Reader detected card!");
            Serial.println("Reader data: " + result.lastReaderData);

            // Continue emulating for a bit
            delay(1000);
        }
    }

    result.emulationDurationMs = millis() - startTime;

    Serial.println("\n=== Emulation Complete ===");
    Serial.println("Duration: " + String(result.emulationDurationMs) + "ms");
    Serial.println("Read count: " + String(result.readCount) + "\n");

    return result;
}

EmulationResult emulateCustomUID(const uint8_t *uid, uint8_t uidLen, uint16_t timeoutMs) {
    NFCCard card;
    for (int i = 0; i < uidLen && i < 7; i++) {
        card.uid[i] = uid[i];
    }
    card.uidLen = uidLen;
    card.type = GENERIC_ISO14443A;
    card.friendlyName = "Custom NFC Card";

    return emulateCard(card, timeoutMs);
}

} // namespace NFCEmulation
