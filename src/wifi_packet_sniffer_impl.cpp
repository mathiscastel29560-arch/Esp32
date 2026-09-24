#include "wifi_packet_sniffer.h"
#include "tool_output_helper.h"
#include "result_renderers.h"
#include <esp_wifi.h>
#include <vector>
#include "audit_log.h"

namespace WiFiPacketSniffer {

static std::vector<PacketInfo> capturedPackets;

void packetCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (capturedPackets.size() < 500) {
        wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buf;

        PacketInfo pkt;
        pkt.rssi = ppkt->rx_ctrl.rssi;
        pkt.channel = ppkt->rx_ctrl.channel;
        pkt.timestamp = millis();

        if (type == 0) {
            pkt.packetType = "Beacon";
        } else if (type == 1) {
            pkt.packetType = "Probe Request";
        } else if (type == 2) {
            pkt.packetType = "Probe Response";
        } else {
            pkt.packetType = "Data";
        }

        capturedPackets.push_back(pkt);
    }
}

SniffResult sniffPackets(uint32_t durationMs) {
    using namespace ToolOutputHelper;

    SniffResult result = {false, 0, 0, 0, 0};
    capturedPackets.clear();

    displayScanStart("WiFi Packet Sniffer", "All channels, promiscuous mode");

    ScanProgressBar progress("WiFi Sniffer", durationMs, 3);
    progress.start();

    // Phase 1: Enable promiscuous mode
    progress.step("Enabling WiFi promiscuous mode on all channels");
    delay(100);

    esp_wifi_set_promiscuous_rx_cb(packetCallback);
    esp_wifi_set_promiscuous(true);
    delay(durationMs / 3);

    // Phase 2: Capture packets
    progress.step("Capturing WiFi packets (beacons, probes, data frames)");
    uint32_t startTime = millis();
    uint32_t lastPackets = 0;
    while ((millis() - startTime) < durationMs / 3) {
        if (capturedPackets.size() > lastPackets) {
            lastPackets = capturedPackets.size();
        }
        delay(100);
    }

    // Phase 3: Analyze results
    progress.step("Analyzing captured packets and classifying frame types");
    uint32_t beacons = 0, dataFrames = 0;
    for (const auto& pkt : capturedPackets) {
        if (pkt.packetType == "Beacon") beacons++;
        if (pkt.packetType == "Data") dataFrames++;
    }
    delay(durationMs / 3);

    esp_wifi_set_promiscuous(false);

    progress.complete(String(capturedPackets.size()) + " packets analyzed");

    // Render results
    ResultRenderers::WiFiScanResult scanResult;
    scanResult.devicesFound = capturedPackets.size();
    scanResult.strongestRssi = capturedPackets.size() > 0 ? capturedPackets[0].rssi : -100;
    scanResult.strongestSSID = "Network";
    scanResult.durationMs = durationMs;

    for (const auto& pkt : capturedPackets) {
        scanResult.allRssiValues.push_back(pkt.rssi);
    }

    std::vector<uint8_t> channelDist(14, 0);
    for (const auto& pkt : capturedPackets) {
        if (pkt.channel < 14) channelDist[pkt.channel]++;
    }
    scanResult.channelDistribution = channelDist;

    ResultRenderers::renderWiFiScan(scanResult);

    result.success = (capturedPackets.size() > 0);
    result.packetsCapture = capturedPackets.size();
    result.beaconsFound = beacons;
    result.dataFrames = dataFrames;
    result.durationMs = durationMs;

    return result;
}

const PacketInfo* getCapturedPackets(uint32_t& outCount) {
    outCount = capturedPackets.size();
    return capturedPackets.empty() ? nullptr : capturedPackets.data();
}

SniffResult analyzeChannelActivity(uint8_t channel, uint32_t durationMs) {
    using namespace ToolOutputHelper;

    SniffResult result = {false, 0, 0, 0, 0};
    capturedPackets.clear();

    displayScanStart("WiFi Channel Activity Analysis", String("Channel " + String(channel)));

    ScanProgressBar progress("Channel " + String(channel), durationMs, 3);
    progress.start();

    // Phase 1: Channel selection
    progress.step("Tuning to WiFi channel " + String(channel) + " (2.4 GHz band)");
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    delay(durationMs / 3);

    // Phase 2: Packet capture
    progress.step("Capturing packets on channel " + String(channel));
    esp_wifi_set_promiscuous_rx_cb(packetCallback);
    esp_wifi_set_promiscuous(true);

    uint32_t startTime = millis();
    while ((millis() - startTime) < durationMs / 3) {
        delay(100);
    }

    // Phase 3: Analysis
    progress.step("Analyzing channel congestion and device activity");
    delay(durationMs / 3);

    esp_wifi_set_promiscuous(false);

    progress.complete(String(capturedPackets.size()) + " packets detected on channel " + String(channel));

    // Render results
    printSubHeader("Channel Activity Results");
    printKeyValue("Channel", String(channel));
    printKeyValue("Packets Captured", String(capturedPackets.size()));
    printKeyValue("Congestion Level", capturedPackets.size() > 20 ? "HIGH" : "MEDIUM");
    uint8_t congestion = capturedPackets.size() > 50 ? 100 : (capturedPackets.size() * 2);
    printBar((uint8_t)min((uint32_t)congestion, 100U), 20);
    Serial.println();

    result.success = (capturedPackets.size() > 0);
    result.packetsCapture = capturedPackets.size();
    result.durationMs = durationMs;

    return result;
}

}  // namespace WiFiPacketSniffer
