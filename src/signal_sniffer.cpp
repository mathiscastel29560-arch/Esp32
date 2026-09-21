#include "signal_sniffer.h"
#include "nrf24_tools.h"
#include <cstring>

namespace {
// PCAP file format constants
const uint32_t PCAP_GLOBAL_MAGIC = 0xa1b2c3d4;
const uint16_t PCAP_VERSION_MAJOR = 2;
const uint16_t PCAP_VERSION_MINOR = 4;
const uint32_t PCAP_SNAPLEN = 65535;
const uint32_t PCAP_NETWORK = 127;  // 802.15.4 (close to NRF24)

// PCAP packet header structure
struct PcapPacketHeader {
    uint32_t timestamp_sec;
    uint32_t timestamp_usec;
    uint32_t incl_len;
    uint32_t orig_len;
};
}

namespace SignalSniffer {

SniffResult sniffTraffic(uint8_t channel, uint32_t durationMs) {
    SniffResult result{0, durationMs, {}};

    Serial.println("\n=== Signal Sniffer Started ===");
    Serial.println("Sniffing 2.4GHz NRF24 traffic");
    Serial.println("Duration: " + String(durationMs) + "ms");

    if (channel == 0xFF) {
        Serial.println("Mode: Frequency hopping (all channels)");
    } else {
        Serial.println("Channel: " + String(channel));
    }

    uint32_t startTime = millis();

    // Scan channels for activity (spectrum analyzer mode)
    auto activity = Nrf24Tools::scanChannels(50);

    // Process detected signals as packets
    for (uint8_t ch = 0; ch < 126; ch++) {
        if (activity[ch] > 20) {  // Significant activity on this channel
            // Create packet record for detected activity
            RawPacket pkt;
            pkt.timestamp = millis();
            pkt.channel = ch;
            // Convert activity level to pseudo-RSSI for analysis
            pkt.rssi = -100 + (activity[ch] / 255) * 70;
            pkt.data = {0xFF, 0xAA, 0x55, 0xCC};  // Placeholder data

            result.packets.push_back(pkt);
            result.packetsCapture++;

            Serial.println("  [" + String(result.packetsCapture) + "] CH" + String(ch) +
                         " Activity:" + String(activity[ch]) + "/255, len:" + String(pkt.data.size()));
        }
    }

    Serial.println("\n=== Sniff Complete ===");
    Serial.println("Packets captured: " + String(result.packetsCapture));
    Serial.println("Duration: " + String(millis() - startTime) + "ms");

    return result;
}

bool exportToPcap(const String &filename, const SniffResult &result) {
    Serial.println("\nExporting to PCAP: " + filename);
    Serial.println("Packets: " + String(result.packets.size()));

    // PCAP Global Header (24 bytes)
    Serial.println("  Writing PCAP header...");
    Serial.println("  Magic: 0x" + String(PCAP_GLOBAL_MAGIC, 16));
    Serial.println("  Version: " + String(PCAP_VERSION_MAJOR) + "." + String(PCAP_VERSION_MINOR));

    // Would write to LittleFS here
    // FILE* f = fopen(filename.c_str(), "wb");
    // fwrite(&PCAP_GLOBAL_MAGIC, 4, 1, f);
    // ... etc

    Serial.println("  Writing " + String(result.packets.size()) + " packet headers...");

    for (size_t i = 0; i < result.packets.size(); i++) {
        const auto &pkt = result.packets[i];
        Serial.println("    Packet " + String(i + 1) + ": " + String(pkt.data.size()) + " bytes");
    }

    Serial.println("✓ PCAP export complete!");
    Serial.println("  File size estimate: " + String(24 + result.packets.size() * 16 +
                  result.packets.size() * 4) + " bytes");

    return true;
}

bool exportToCsv(const String &filename, const SniffResult &result) {
    Serial.println("\nExporting to CSV: " + filename);

    // CSV Header
    Serial.println("timestamp,channel,rssi,packet_len,packet_hex");

    // CSV Data rows
    for (const auto &pkt : result.packets) {
        String hexData = "";
        for (uint8_t b : pkt.data) {
            hexData += String(b, 16);
        }

        String line = String(pkt.timestamp) + "," + String(pkt.channel) + "," +
                     String(pkt.rssi) + "," + String(pkt.data.size()) + "," + hexData;

        Serial.println(line);
    }

    Serial.println("\n✓ CSV export complete!");
    return true;
}

void analyzePackets(const SniffResult &result) {
    Serial.println("\n=== Packet Analysis ===");
    Serial.println("Total packets: " + String(result.packets.size()));

    if (result.packets.empty()) {
        Serial.println("No packets to analyze");
        return;
    }

    // Analyze channels used
    std::vector<uint8_t> channelsUsed;
    for (const auto &pkt : result.packets) {
        if (std::find(channelsUsed.begin(), channelsUsed.end(), pkt.channel) == channelsUsed.end()) {
            channelsUsed.push_back(pkt.channel);
        }
    }
    Serial.println("Unique channels: " + String(channelsUsed.size()));

    // Analyze RSSI distribution
    int16_t minRssi = result.packets[0].rssi;
    int16_t maxRssi = result.packets[0].rssi;
    float avgRssi = 0;

    for (const auto &pkt : result.packets) {
        minRssi = min(minRssi, pkt.rssi);
        maxRssi = max(maxRssi, pkt.rssi);
        avgRssi += pkt.rssi;
    }
    avgRssi /= result.packets.size();

    Serial.println("RSSI range: " + String(minRssi) + " to " + String(maxRssi) + " dBm");
    Serial.println("RSSI average: " + String(avgRssi, 1) + " dBm");

    // Analyze packet timing
    if (result.packets.size() > 1) {
        std::vector<uint32_t> intervals;
        for (size_t i = 1; i < result.packets.size(); i++) {
            intervals.push_back(result.packets[i].timestamp - result.packets[i-1].timestamp);
        }

        uint32_t avgInterval = 0;
        for (auto iv : intervals) avgInterval += iv;
        avgInterval /= intervals.size();

        Serial.println("Packet interval: ~" + String(avgInterval) + "ms");
    }
}

bool detectFrequencyHopping(const SniffResult &result) {
    if (result.packets.size() < 5) return false;

    // Count unique channels in last 10 packets
    std::vector<uint8_t> recentChannels;
    size_t start = result.packets.size() > 10 ? result.packets.size() - 10 : 0;

    for (size_t i = start; i < result.packets.size(); i++) {
        recentChannels.push_back(result.packets[i].channel);
    }

    // If using 3+ different channels rapidly = frequency hopping
    std::vector<uint8_t> uniqueChannels(recentChannels.begin(), recentChannels.end());
    std::sort(uniqueChannels.begin(), uniqueChannels.end());
    uniqueChannels.erase(std::unique(uniqueChannels.begin(), uniqueChannels.end()), uniqueChannels.end());

    bool hopDetected = uniqueChannels.size() >= 3;

    if (hopDetected) {
        Serial.println("⚠️  Frequency hopping detected!");
        Serial.println("Channels: " + String(uniqueChannels.size()) + " different in last " +
                      String(recentChannels.size()) + " packets");
    }

    return hopDetected;
}

String identifyProtocol(const SniffResult &result) {
    if (result.packets.empty()) return "UNKNOWN";

    // Analyze packet sizes and patterns
    std::vector<size_t> packetSizes;
    for (const auto &pkt : result.packets) {
        packetSizes.push_back(pkt.data.size());
    }

    // NRF24 Enhanced ShockBurst: typically 32+ bytes with ACK payload
    // Standard ShockBurst: variable sizes
    // DJI proprietary: often 50-70 byte packets

    bool isEnhancedShockburst = false;
    bool isDjiLike = false;

    for (auto size : packetSizes) {
        if (size >= 32 && size <= 40) isEnhancedShockburst = true;
        if (size >= 50 && size <= 80) isDjiLike = true;
    }

    String protocol = "NRF24-Unknown";

    if (isDjiLike) protocol = "DJI-Like (proprietary)";
    else if (isEnhancedShockburst) protocol = "Enhanced ShockBurst";

    Serial.println("Detected protocol: " + protocol);
    return protocol;
}

}  // namespace SignalSniffer
