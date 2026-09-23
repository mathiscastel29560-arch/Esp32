#ifndef ZIGBEE_SNIFFER_H
#define ZIGBEE_SNIFFER_H

#include <Arduino.h>

namespace ZigbeeSniffer {
    struct ZigbeeFrame {
        uint32_t timestamp;
        int8_t rssi;
        uint8_t channel;
        uint16_t pan_id;
        uint16_t src_addr;
        uint16_t dst_addr;
        uint8_t seq_num;
        uint8_t payload_len;
        uint8_t payload[127];
    };

    bool begin();
    uint32_t sniff(uint32_t duration_seconds = 30);
    uint32_t sniffChannel(uint8_t channel, uint32_t duration_seconds = 30);
    bool getLastFrame(ZigbeeFrame& frame);
    String decodeFrame(const ZigbeeFrame& frame);
    
    struct Stats {
        uint32_t total_frames;
        uint32_t valid_frames;
        uint32_t pan_ids_found;
        int8_t min_rssi;
        int8_t max_rssi;
    };
    Stats getStats();
    void end();
}

#endif
