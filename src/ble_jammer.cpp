#include "ble_jammer.h"
#include "audit_log.h"
#include <RF24.h>

namespace BleJammer {
    static RF24* radio = nullptr;
    static bool jamming = false;
    static Stats stats = {0};

    bool begin() {
        if (!radio) {
            radio = new RF24(15, 14);
        }

        if (!radio->begin()) {
            Serial.println("[BleJammer] RF24 init failed");
            return false;
        }

        radio->setDataRate(RF24_1MBPS);
        radio->setPALevel(RF24_PA_HIGH);
        radio->startListening();

        AuditLog::instance().log(AuditEventType::TOOL_START, "BleJammer", 
                                "BLE jamming initialized");
        return true;
    }

    bool startJamming(uint32_t duration_ms) {
        if (!radio || jamming) return false;

        jamming = true;
        uint32_t start_time = millis();
        uint8_t noise[32];
        memset(noise, 0xAA, sizeof(noise));  // Valid BLE preamble to trigger receivers

        Serial.printf("[BleJammer] Starting jamming for %u ms\n", duration_ms);

        // Jam all BLE advertising channels (37, 38, 39)
        uint8_t ble_channels[] = {37, 38, 39};

        while (millis() - start_time < duration_ms && jamming) {
            for (uint8_t i = 0; i < 3; i++) {
                radio->setChannel(ble_channels[i] + 2);  // Map to 2.4GHz
                radio->stopListening();
                radio->write(noise, sizeof(noise));
                stats.packets_sent++;
            }
            delay(10);
        }

        jamming = false;
        stats.jam_duration_ms += (millis() - start_time);
        stats.channels_targeted = 3;

        char details[96];
        snprintf(details, sizeof(details), "Sent %u jam packets on 3 BLE channels",
                stats.packets_sent);
        AuditLog::instance().log(AuditEventType::TOOL_SUCCESS, "BleJammer", details);

        return true;
    }

    bool stopJamming() {
        jamming = false;
        return true;
    }

    bool isJamming() {
        return jamming;
    }

    Stats getStats() {
        return stats;
    }
}
