#include "garage_door_cloner.h"
#include "audit_log.h"
#include <LittleFS.h>

namespace GarageDoorCloner {
    static Stats stats = {0};
    static GarageDoorCode last_captured = {0};

    bool begin() {
        if (!LittleFS.begin()) {
            Serial.println("[GarageDoorCloner] LittleFS init failed");
            return false;
        }

        if (!LittleFS.exists("/garage_codes")) {
            LittleFS.mkdir("/garage_codes");
        }

        AuditLog::instance().log(AuditEventType::TOOL_START, "GarageDoorCloner", 
                                "Garage door cloner initialized");
        return true;
    }

    bool captureCode(uint32_t timeout_ms, GarageDoorCode& code) {
        Serial.printf("[GarageDoorCloner] Listening for 433MHz code (timeout: %u ms)...\n", 
                     timeout_ms);
        
        uint32_t start_time = millis();
        bool found = false;

        // Simulate code capture (in real deployment, use CC1101 driver)
        while (millis() - start_time < timeout_ms) {
            // Simulated detection of 433MHz signal
            if (random(100) < 5) {  // 5% chance to detect signal
                code.timestamp = millis();
                code.frequency = 433920000;  // 433.92MHz
                code.code = random(0xFFFFFF);
                code.length_bits = 24;
                code.rssi = -60 + random(20);
                last_captured = code;
                stats.codes_captured++;
                found = true;
                break;
            }
            delay(100);
        }

        if (found) {
            Serial.printf("[GarageDoorCloner] Code captured: 0x%06X\n", code.code);
            AuditLog::instance().log(AuditEventType::DEVICE_FOUND, "GarageDoorCloner", 
                                    "433MHz garage door code detected");
        }

        return found;
    }

    bool storeCode(const char* name, const GarageDoorCode& code) {
        if (!name) return false;

        String filepath = "/garage_codes/" + String(name) + ".bin";
        File f = LittleFS.open(filepath, FILE_WRITE);
        if (!f) return false;

        f.write((uint8_t*)&code, sizeof(GarageDoorCode));
        f.close();

        Serial.printf("[GarageDoorCloner] Code stored: %s\n", name);
        return true;
    }

    bool replayCode(const GarageDoorCode& code, uint8_t repeat_count) {
        Serial.printf("[GarageDoorCloner] Replaying code 0x%06X (%u times)...\n", 
                     code.code, repeat_count);

        for (uint8_t i = 0; i < repeat_count; i++) {
            // In real deployment: transmit via CC1101 on 433MHz
            delay(50);
            stats.codes_replayed++;
        }

        char details[96];
        snprintf(details, sizeof(details), "Replayed code 0x%06X (%u times)", 
                code.code, repeat_count);
        AuditLog::instance().log(AuditEventType::ATTACK_INITIATED, "GarageDoorCloner", details);

        return true;
    }

    String listStoredCodes() {
        String result = "Stored codes:\n";
        File root = LittleFS.open("/garage_codes");
        if (!root) return result + "  (none)";

        File file = root.openNextFile();
        uint32_t count = 0;
        while (file) {
            if (!file.isDirectory()) {
                result += "  - " + String(file.name()) + " (" + String(file.size()) + " bytes)\n";
                count++;
            }
            file = root.openNextFile();
        }

        if (count == 0) result += "  (none)";
        return result;
    }

    Stats getStats() {
        return stats;
    }
}
