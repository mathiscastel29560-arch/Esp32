#include "rfid_cloner.h"
#include "audit_log.h"
#include <LittleFS.h>

namespace RfidCloner {
    static Stats stats = {0};

    bool begin() {
        if (!LittleFS.begin()) return false;
        if (!LittleFS.exists("/rfid_cards")) LittleFS.mkdir("/rfid_cards");

        AuditLog::instance().log(AuditEventType::TOOL_START, "RfidCloner", 
                                "RFID cloner initialized");
        return true;
    }

    bool scanCard(RfidCard& card, uint32_t timeout_ms) {
        Serial.println("[RfidCloner] Scanning for RFID card...");
        uint32_t start = millis();

        while (millis() - start < timeout_ms) {
            // In real: use PN532 driver to read RFID
            if (random(100) < 10) {  // Simulated detection
                card.uid_len = 4;
                for (uint8_t i = 0; i < card.uid_len; i++) {
                    card.uid[i] = random(256);
                }
                strcpy(card.card_type, "ISO14443A");
                card.timestamp = millis();
                stats.cards_scanned++;

                Serial.printf("[RfidCloner] Card found: ");
                for (uint8_t i = 0; i < card.uid_len; i++) {
                    Serial.printf("%02X ", card.uid[i]);
                }
                Serial.println();

                AuditLog::instance().log(AuditEventType::DEVICE_FOUND, "RfidCloner", 
                                        card.card_type);
                return true;
            }
            delay(100);
        }

        return false;
    }

    bool storeCard(const char* name, const RfidCard& card) {
        String filepath = "/rfid_cards/" + String(name) + ".bin";
        File f = LittleFS.open(filepath, FILE_WRITE);
        if (!f) return false;

        f.write((uint8_t*)&card, sizeof(RfidCard));
        f.close();
        stats.cards_cloned++;

        return true;
    }

    bool emulateCard(const RfidCard& card) {
        Serial.print("[RfidCloner] Emulating card: ");
        for (uint8_t i = 0; i < card.uid_len; i++) {
            Serial.printf("%02X ", card.uid[i]);
        }
        Serial.println();

        stats.emulations++;
        AuditLog::instance().log(AuditEventType::ATTACK_INITIATED, "RfidCloner", 
                                "Card emulation started");
        return true;
    }

    String listStoredCards() {
        String result = "Stored cards:\n";
        File root = LittleFS.open("/rfid_cards");
        if (!root) return result + "  (none)";

        File file = root.openNextFile();
        while (file) {
            if (!file.isDirectory()) {
                result += "  - " + String(file.name()) + "\n";
            }
            file = root.openNextFile();
        }
        return result;
    }

    Stats getStats() {
        return stats;
    }
}
