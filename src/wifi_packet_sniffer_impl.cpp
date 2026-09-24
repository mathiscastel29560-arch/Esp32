#include "wifi_packet_sniffer.h"
#include <esp_wifi.h>
#include <vector>

namespace WiFiPacketSniffer {

static std::vector<PacketInfo> capturedPackets;

void packetCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (capturedPackets.size() < 500) {  // Limit memory
        wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buf;
        
        PacketInfo pkt;
        pkt.rssi = ppkt->rx_ctrl.rssi;
        pkt.channel = ppkt->rx_ctrl.channel;
        pkt.timestamp = millis();
        
        if (type == 0) {  // BEACON
            pkt.packetType = "Beacon";
        } else if (type == 1) {  // PROBE_REQ
            pkt.packetType = "Probe Request";
        } else if (type == 2) {  // PROBE_RESP
            pkt.packetType = "Probe Response";
        } else {
            pkt.packetType = "Data";
        }
        
        capturedPackets.push_back(pkt);
    }
}

SniffResult sniffPackets(uint32_t durationMs) {
    SniffResult result = {false, 0, 0, 0, 0};
    capturedPackets.clear();
    uint32_t startTime = millis();

    Serial.println("\n=== WiFi Packet Sniffer (REAL Promiscuous Mode) ===");
    Serial.printf("Duration: %lums\n", durationMs);

    // Enable promiscuous mode
    delay(100);
    
    esp_wifi_set_promiscuous_rx_cb(packetCallback);
    esp_wifi_set_promiscuous(true);

    uint32_t lastPackets = 0;
    while ((millis() - startTime) < durationMs) {
        if (capturedPackets.size() > lastPackets) {
            Serial.printf("✓ Captured: %u packets\r", (uint32_t)capturedPackets.size());
            lastPackets = capturedPackets.size();
        }
        delay(100);
    }

    // Analyze captured packets
    uint32_t beacons = 0, dataFrames = 0;
    for (const auto& pkt : capturedPackets) {
        if (pkt.packetType == "Beacon") beacons++;
        if (pkt.packetType == "Data") dataFrames++;
    }

    esp_wifi_set_promiscuous(false);

    result.success = (capturedPackets.size() > 0);
    result.packetsCapture = capturedPackets.size();
    result.beaconsFound = beacons;
    result.dataFrames = dataFrames;
    result.durationMs = millis() - startTime;

    Serial.printf("\n✓ Capture complete: %u packets (Beacons: %u, Data: %u)\n", 
                 result.packetsCapture, beacons, dataFrames);

    return result;
}

const PacketInfo* getCapturedPackets(uint32_t& outCount) {
    outCount = capturedPackets.size();
    return capturedPackets.empty() ? nullptr : capturedPackets.data();
}

SniffResult analyzeChannelActivity(uint8_t channel, uint32_t durationMs) {
    SniffResult result = {false, 0, 0, 0, 0};
    uint32_t startTime = millis();

    Serial.println("\n=== Channel Activity Analysis ===");
    Serial.printf("Channel: %u | Duration: %lums\n", channel, durationMs);

    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    capturedPackets.clear();

    esp_wifi_set_promiscuous_rx_cb(packetCallback);
    esp_wifi_set_promiscuous(true);

    while ((millis() - startTime) < durationMs) {
        delay(100);
    }

    esp_wifi_set_promiscuous(false);

    result.success = (capturedPackets.size() > 0);
    result.packetsCapture = capturedPackets.size();
    result.durationMs = millis() - startTime;

    Serial.printf("✓ Channel %u: %u packets in %lums\n", channel, result.packetsCapture, result.durationMs);

    return result;
}

}  // namespace WiFiPacketSniffer
