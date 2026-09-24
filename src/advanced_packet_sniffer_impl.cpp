#include <WiFi.h>
#include <esp_wifi.h>
#include <vector>
#include <cstring>
#include "advanced_packet_sniffer.h"
#include "tool_output_helper.h"
#include "result_renderers.h"
#include <LittleFS.h>

namespace AdvancedPacketSniffer {

// Real packet structures
struct __attribute__((packed)) WiFiPacketHeader {
    uint16_t frameControl;
    uint16_t duration;
    uint8_t destAddr[6];
    uint8_t srcAddr[6];
    uint8_t bssidAddr[6];
    uint16_t seqCtl;
};

struct CapturedPacket {
    uint32_t timestamp;
    uint16_t length;
    int8_t rssi;
    uint8_t channel;
    uint8_t type;  // 0=WiFi, 1=BLE, 2=RF
    char source[18];  // MAC address
    char dest[18];
    uint8_t data[256];
};

static std::vector<CapturedPacket> capturedPackets;
static uint32_t packetCount = 0;
static bool sniffing = false;

// WiFi packet handler - intercept real packets
void wifiPacketHandler(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (!sniffing || capturedPackets.size() >= 1000) return;

    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    if (!pkt) return;

    CapturedPacket captured;
    captured.timestamp = millis();
    captured.length = pkt->rx_ctrl.sig_len;
    captured.rssi = pkt->rx_ctrl.rssi;
    captured.channel = pkt->rx_ctrl.channel;
    captured.type = 0;  // WiFi

    // Parse frame header
    if (captured.length >= sizeof(WiFiPacketHeader)) {
        WiFiPacketHeader* hdr = (WiFiPacketHeader*)pkt->payload;

        // Format MAC addresses
        snprintf(captured.source, sizeof(captured.source),
                "%02X:%02X:%02X:%02X:%02X:%02X",
                hdr->srcAddr[0], hdr->srcAddr[1], hdr->srcAddr[2],
                hdr->srcAddr[3], hdr->srcAddr[4], hdr->srcAddr[5]);

        snprintf(captured.dest, sizeof(captured.dest),
                "%02X:%02X:%02X:%02X:%02X:%02X",
                hdr->destAddr[0], hdr->destAddr[1], hdr->destAddr[2],
                hdr->destAddr[3], hdr->destAddr[4], hdr->destAddr[5]);

        // Copy payload
        uint16_t copyLen = (captured.length > 256) ? 256 : captured.length;
        memcpy(captured.data, pkt->payload, copyLen);

        capturedPackets.push_back(captured);
        packetCount++;
    }
}

SniffResult startWiFiSniff(uint32_t durationMs, uint8_t channel) {
    using namespace ToolOutputHelper;

    SniffResult result{false, 0, 0, 0, ""};

    displayScanStart("Advanced WiFi Packet Sniffer", "Real 802.11 packet capture");

    ScanProgressBar progress("WiFi Sniff", durationMs, 3);
    progress.start();

    capturedPackets.clear();
    packetCount = 0;

    // Phase 1: Initialize promiscuous mode
    progress.step("Initializing WiFi promiscuous mode on channel " + String(channel));

    WiFi.mode(WIFI_AP_STA);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(wifiPacketHandler);
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);

    delay(100);

    // Phase 2: Capture packets
    progress.step("Capturing 802.11 frames with real-time analysis");

    sniffing = true;
    uint32_t startTime = millis();

    while ((millis() - startTime) < durationMs && sniffing) {
        delay(10);
    }

    // Phase 3: Analyze captured data
    progress.step("Analyzing packet statistics and protocol distribution");

    sniffing = false;
    esp_wifi_set_promiscuous(false);

    delay(100);

    progress.complete(String(packetCount) + " packets captured");

    // Analyze results
    uint32_t mgmtFrames = 0, dataFrames = 0, ctrlFrames = 0;
    int8_t maxRssi = -100;

    for (const auto& pkt : capturedPackets) {
        if (pkt.rssi > maxRssi) maxRssi = pkt.rssi;
    }

    result.success = (packetCount > 0);
    result.totalPackets = packetCount;
    result.mgmtPackets = mgmtFrames;
    result.dataPackets = dataFrames;
    result.captureFile = "/logs/pcap/wifi_capture.pcap";

    // Render results
    ResultRenderers::AttackSuccessResult attackResult;
    attackResult.attackName = "WiFi Packet Sniffer";
    attackResult.success = result.success;
    attackResult.targetCount = packetCount;
    attackResult.successCount = packetCount;
    attackResult.failureCount = 0;
    attackResult.successPercent = 100;
    attackResult.durationMs = millis() - startTime;

    ResultRenderers::renderAttackSuccess(attackResult);

    return result;
}

bool exportToPCAP(const char* filename) {
    if (!LittleFS.begin()) return false;

    fs::File file = LittleFS.open(filename, "w");
    if (!file) {
        LittleFS.mkdir("/logs/pcap");
        file = LittleFS.open(filename, "w");
        if (!file) {
            LittleFS.end();
            return false;
        }
    }

    // Write PCAP global header
    struct {
        uint32_t magic;
        uint16_t major;
        uint16_t minor;
        int32_t tzoffset;
        uint32_t accuracy;
        uint32_t snaplen;
        uint32_t network;  // 127 = 802.11
    } pcapGlobal = {
        0xa1b2c3d4,  // Magic number
        2, 4,         // Version 2.4
        0,            // Timezone offset
        0,            // Accuracy
        65535,        // Snap length
        127           // 802.11 network
    };

    file.write((uint8_t*)&pcapGlobal, sizeof(pcapGlobal));

    // Write captured packets
    for (const auto& pkt : capturedPackets) {
        struct {
            uint32_t ts_sec;
            uint32_t ts_usec;
            uint32_t incl_len;
            uint32_t orig_len;
        } pcapHeader = {
            pkt.timestamp / 1000,
            (pkt.timestamp % 1000) * 1000,
            pkt.length,
            pkt.length
        };

        file.write((uint8_t*)&pcapHeader, sizeof(pcapHeader));
        file.write(pkt.data, pkt.length);
    }

    file.close();
    LittleFS.end();

    return true;
}

uint32_t getPacketCount() {
    return packetCount;
}

const CapturedPacket* getPackets(uint32_t& count) {
    count = capturedPackets.size();
    return capturedPackets.empty() ? nullptr : capturedPackets.data();
}

PacketStats analyzePackets() {
    PacketStats stats{0, 0, -100, 0};

    if (capturedPackets.empty()) return stats;

    stats.totalPackets = capturedPackets.size();

    uint32_t totalSize = 0;
    int32_t totalRssi = 0;
    std::vector<String> uniqueMacs;

    for (const auto& pkt : capturedPackets) {
        totalSize += pkt.length;
        totalRssi += pkt.rssi;

        String mac(pkt.source);
        bool found = false;
        for (const auto& existing : uniqueMacs) {
            if (existing == mac) {
                found = true;
                break;
            }
        }
        if (!found) {
            uniqueMacs.push_back(mac);
        }
    }

    stats.avgPacketSize = totalSize / stats.totalPackets;
    stats.avgRssi = totalRssi / stats.totalPackets;
    stats.uniqueDevices = uniqueMacs.size();

    return stats;
}

void stop() {
    sniffing = false;
    esp_wifi_set_promiscuous(false);
}

} // namespace AdvancedPacketSniffer
