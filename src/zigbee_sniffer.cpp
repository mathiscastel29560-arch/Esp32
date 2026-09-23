#include "zigbee_sniffer.h"
#include "audit_log.h"
#include <RF24.h>

namespace ZigbeeSniffer {
    static RF24* radio = nullptr;
    static ZigbeeFrame last_frame = {0};
    static Stats stats = {0};

    bool begin() {
        if (!radio) {
            radio = new RF24(15, 14);  // CE=15, CSN=14 (NRF24 pins)
        }

        if (!radio->begin()) {
            Serial.println("[ZigbeeSniffer] RF24 initialization failed");
            AuditLog::instance().log(AuditEventType::ERROR_OCCURRED, "ZigbeeSniffer", 
                                    "RF24 init failed");
            return false;
        }

        radio->setDataRate(RF24_250KBPS);  // Zigbee uses 250kbps
        radio->setPALevel(RF24_PA_HIGH);
        radio->setChannel(76);  // Zigbee channel 15 (2.4GHz)
        radio->startListening();

        memset(&stats, 0, sizeof(stats));
        
        AuditLog::instance().log(AuditEventType::TOOL_START, "ZigbeeSniffer", 
                                "Initialized for 802.15.4 sniffing");
        Serial.println("[ZigbeeSniffer] Ready to sniff Zigbee traffic");
        return true;
    }

    uint32_t sniff(uint32_t duration_seconds) {
        if (!radio) return 0;

        uint32_t start_time = millis();
        uint32_t frame_count = 0;

        Serial.printf("[ZigbeeSniffer] Sniffing for %u seconds...\n", duration_seconds);

        while (millis() - start_time < duration_seconds * 1000) {
            if (radio->available()) {
                uint8_t len = radio->getDynamicPayloadSize();
                if (len > 0 && len <= 127) {
                    radio->read(last_frame.payload, len);
                    last_frame.payload_len = len;
                    last_frame.timestamp = millis();
                    last_frame.rssi = -40;  // Simplified RSSI
                    last_frame.channel = radio->getChannel();

                    // Parse simplified Zigbee header
                    if (len >= 5) {
                        last_frame.pan_id = (last_frame.payload[3] << 8) | last_frame.payload[2];
                        last_frame.src_addr = (last_frame.payload[5] << 8) | last_frame.payload[4];
                        stats.total_frames++;
                        stats.valid_frames++;
                        frame_count++;

                        Serial.printf("[ZigbeeSniffer] Frame %u: PAN=%04X SRC=%04X Len=%u\n",
                                    frame_count, last_frame.pan_id, last_frame.src_addr, len);
                    }
                }
            }
            delay(10);
        }

        char details[96];
        snprintf(details, sizeof(details), "Captured %u frames, PAN IDs: %u", 
                frame_count, stats.pan_ids_found);
        AuditLog::instance().log(AuditEventType::TOOL_SUCCESS, "ZigbeeSniffer", details);

        return frame_count;
    }

    uint32_t sniffChannel(uint8_t channel, uint32_t duration_seconds) {
        if (!radio) return 0;
        
        radio->setChannel(channel);
        return sniff(duration_seconds);
    }

    bool getLastFrame(ZigbeeFrame& frame) {
        if (last_frame.payload_len == 0) return false;
        frame = last_frame;
        return true;
    }

    String decodeFrame(const ZigbeeFrame& frame) {
        String result = "Zigbee Frame:\n";
        result += "  PAN ID: 0x" + String(frame.pan_id, HEX) + "\n";
        result += "  Src: 0x" + String(frame.src_addr, HEX) + "\n";
        result += "  Dst: 0x" + String(frame.dst_addr, HEX) + "\n";
        result += "  Payload: ";
        for (uint8_t i = 0; i < frame.payload_len; i++) {
            result += String(frame.payload[i], HEX) + " ";
        }
        result += "\n";
        return result;
    }

    Stats getStats() {
        return stats;
    }

    void end() {
        if (radio) {
            radio->stopListening();
        }
    }
}
