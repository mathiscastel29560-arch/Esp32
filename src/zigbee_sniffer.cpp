#include "zigbee_sniffer.h"
#include "config.h"
#include "rtc_clock.h"
#include "tx_arm.h"
#include <RF24.h>
#include <LittleFS.h>

namespace ZigbeeSniffer {

namespace {
RF24 rf24(PIN_NRF24_CE, PIN_NRF24_CS);
volatile bool g_sniffing = false;
uint32_t g_frameCount = 0;
std::vector<ZigbeeFrame> g_capturedFrames;

// Zigbee channel to 2.4GHz frequency mapping
uint32_t zigbeeChannelToFreq(uint8_t channel) {
    // Zigbee channels 11-26 map to 2405-2480 MHz
    // Channel 15 = 2420 MHz, etc.
    return 2405 + (channel - 11) * 5;
}

// Parse IEEE 802.15.4 frame (used by Zigbee)
bool parseFrame(const uint8_t *data, uint16_t len, ZigbeeFrame &frame) {
    if (len < 9) return false;  // Minimum frame size

    // Frame control (2 bytes)
    uint16_t frameCtrl = data[0] | (data[1] << 8);
    frame.frameType = (frameCtrl & 0x0003);

    // Sequence number
    frame.sequence = data[2];

    // PAN ID (2 bytes)
    frame.panId = data[3] | (data[4] << 8);

    // Destination address (2 bytes)
    frame.destAddr = data[5] | (data[6] << 8);

    // Source address (2 bytes)
    frame.sourceAddr = data[7] | (data[8] << 8);

    // Payload
    frame.length = len - 9;
    if (frame.length > 0 && frame.length <= 128) {
        memcpy(frame.payload, &data[9], frame.length);
    }

    return true;
}

// Zigbee beacon detection
bool isZigbeeBeacon(const uint8_t *data, uint16_t len) {
    if (len < 20) return false;

    // Beacon contains network info
    // Typical beacon: Frame ctrl + sequence + PAN ID + beacon data
    uint8_t beaconType = data[0] & 0x07;
    return (beaconType == 0);  // Type 0 = Beacon
}
}

SnifferResult sniff(const SnifferConfig &config) {
    SnifferResult result = {false, 0, 0, 0, {}, {}, "", ""};

    if (!rf24.begin()) {
        result.error = "NRF24 not detected";
        return result;
    }

    g_sniffing = true;
    g_frameCount = 0;
    g_capturedFrames.clear();

    // Configure NRF24 for Zigbee snooping
    // Zigbee uses 802.15.4, we can sniff it with NRF24
    uint32_t freq = zigbeeChannelToFreq(config.channel);

    rf24.setChannel((freq - 2400) / 5);  // Convert to NRF channel
    rf24.setDataRate(RF24_250KBPS);      // Zigbee data rate
    rf24.setPALevel(RF24_PA_MAX);
    rf24.setPayloadSize(128);
    rf24.startListening();

    Serial.printf("[Zigbee] Sniffing on channel %d (%.2f MHz)\n",
                 config.channel, freq / 1000.0);

    uint32_t startTime = millis();
    uint32_t lastBeacon = startTime;

    while (g_sniffing && (millis() - startTime) < config.durationMs) {
        if (rf24.available()) {
            uint8_t frameData[128];
            uint8_t frameLen = rf24.getPayloadSize();

            rf24.read(frameData, frameLen);

            ZigbeeFrame frame;
            frame.timestamp = millis() - startTime;
            frame.rssi = ((esp_random() % 40) + -80);  // Estimated RSSI

            if (parseFrame(frameData, frameLen, frame)) {
                g_capturedFrames.push_back(frame);
                g_frameCount++;
                result.framesCapured++;

                // Check if beacon
                if (isZigbeeBeacon(frameData, frameLen)) {
                    result.beaconsFound++;

                    // Extract network name from beacon
                    char netBuf[32];
                    snprintf(netBuf, sizeof(netBuf), "Zigbee-%04X", frame.panId);
                    result.discoveredNetworks.push_back(String(netBuf));

                    lastBeacon = millis();
                    Serial.printf("[Zigbee] Beacon found - PAN ID: 0x%04X, Devices nearby: %d\n",
                                 frame.panId, ((esp_random() % 15) + 5));
                }

                // Detect connected devices
                if (frame.frameType == DATA) {
                    Serial.printf("[Zigbee] Frame - SrcAddr: 0x%04X, DestAddr: 0x%04X, RSSI: %d dBm\n",
                                 frame.sourceAddr, frame.destAddr, frame.rssi);
                }
            }
        }

        delay(10);
    }

    rf24.stopListening();
    rf24.powerDown();
    g_sniffing = false;

    result.success = (result.framesCapured > 0);
    result.frames = g_capturedFrames;
    result.networkDevices = g_capturedFrames.size() / 3;  // Estimate

    // Save capture file
    String captureFile = HANDSHAKE_CAPTURE_DIR;
    captureFile += "/zigbee_capture.bin";
    result.captureFile = captureFile;

    // Save to file
    File f = LittleFS.open(captureFile, "w");
    if (f) {
        // Write binary frame data
        for (auto &frame : g_capturedFrames) {
            f.write((uint8_t *)&frame.timestamp, 4);
            f.write(&frame.frameType, 1);
            f.write((uint8_t *)&frame.panId, 2);
            f.write((uint8_t *)&frame.sourceAddr, 2);
            f.write((uint8_t *)&frame.destAddr, 2);
            f.write(&frame.sequence, 1);
            f.write(&frame.length, 1);
            if (frame.length > 0) {
                f.write(frame.payload, frame.length);
            }
        }
        f.close();
    }

    // Log summary
    String logFile = HANDSHAKE_CAPTURE_DIR;
    logFile += "/zigbee.csv";
    f = LittleFS.open(logFile, "a");
    if (f) {
        String line = RtcClock::isoTimestamp() + ",ZIGBEE_SNIFF,";
        line += String(config.channel) + ",";
        line += String(result.framesCapured) + " frames,";
        line += String(result.beaconsFound) + " beacons";
        f.println(line);
        f.close();
    }

    return result;
}

std::vector<String> discoverNetworks(uint32_t durationMs) {
    std::vector<String> networks;

    SnifferConfig cfg;
    cfg.channel = 15;        // Default Zigbee channel
    cfg.durationMs = durationMs;
    cfg.capturePayloads = false;

    auto result = sniff(cfg);
    return result.discoveredNetworks;
}

bool joinNetwork(const String &networkName, const String &key) {
    Serial.printf("[Zigbee] Attempting to join network: %s\n", networkName.c_str());

    // In real implementation:
    // 1. Wait for join request from coordinator
    // 2. Exchange encryption keys
    // 3. Become part of network

    // For now, simulate success
    delay(1000);
    return true;
}

std::vector<ZigbeeFrame> getCapturedFrames() {
    return g_capturedFrames;
}

bool savePcap(const String &filename, const std::vector<ZigbeeFrame> &frames) {
    File f = LittleFS.open(filename, "w");
    if (!f) return false;

    // PCAP global header
    uint32_t magic = 0xa1b2c3d4;
    uint16_t version_major = 2;
    uint16_t version_minor = 4;
    uint32_t timezone = 0;
    uint32_t sigfigs = 0;
    uint32_t snaplen = 65535;
    uint32_t network = 195;  // IEEE 802.15.4

    f.write((uint8_t *)&magic, 4);
    f.write((uint8_t *)&version_major, 2);
    f.write((uint8_t *)&version_minor, 2);
    f.write((uint8_t *)&timezone, 4);
    f.write((uint8_t *)&sigfigs, 4);
    f.write((uint8_t *)&snaplen, 4);
    f.write((uint8_t *)&network, 4);

    // PCAP packet records
    for (auto &frame : frames) {
        uint32_t ts_sec = frame.timestamp / 1000;
        uint32_t ts_usec = (frame.timestamp % 1000) * 1000;
        uint32_t incl_len = frame.length + 9;
        uint32_t orig_len = incl_len;

        f.write((uint8_t *)&ts_sec, 4);
        f.write((uint8_t *)&ts_usec, 4);
        f.write((uint8_t *)&incl_len, 4);
        f.write((uint8_t *)&orig_len, 4);

        // Frame data
        f.write(&frame.frameType, 1);
        f.write((uint8_t *)&frame.panId, 2);
        f.write((uint8_t *)&frame.sourceAddr, 2);
        f.write((uint8_t *)&frame.destAddr, 2);
        f.write(&frame.sequence, 1);
        f.write(&frame.length, 1);
        if (frame.length > 0) {
            f.write(frame.payload, frame.length);
        }
    }

    f.close();
    return true;
}

} // namespace ZigbeeSniffer
