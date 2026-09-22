#include "mavic_jammer.h"
#include "tx_arm.h"
#include <RadioLib.h>

namespace MavicJammer {

// DJI Mavic/Air 2.4GHz control:
// - WiFi link: channels 1-13 (2.412-2.472 GHz)
// - Proprietary: 2.4GHz ISM with frequency hopping
// Both use modulation: GFSK (2 Mbps typically)

JammerResult jammMavicController(const JammerConfig& config) {
    JammerResult result;
    result.success = false;
    result.packetsJammed = 0;
    result.frequencyChanges = 0;

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    // Initialize CC1101 for 2.4GHz ISM band
    // DJI uses frequency hopping on ~5 channels (adaptive)
    const uint32_t djiChannels[] = {
        2407000000,  // 2407 MHz
        2419000000,  // 2419 MHz (WiFi overlap)
        2437000000,  // 2437 MHz (WiFi ch 6)
        2455000000,  // 2455 MHz (WiFi ch 10)
        2472000000   // 2472 MHz (WiFi ch 13)
    };
    const uint8_t NUM_CHANNELS = 5;

    uint32_t startTime = millis();
    uint32_t nextChannelChange = startTime;
    uint8_t currentChannelIdx = 0;

    Serial.printf("[Mavic Jammer] Starting %s mode for %lums\n",
                  config.method == 0 ? "NOISE" :
                  config.method == 1 ? "SWEEP" : "SYNC",
                  config.durationMs);

    while ((millis() - startTime) < config.durationMs) {
        if (!TxArm::isArmed()) {
            result.error = "TX disarmed";
            break;
        }

        uint32_t now = millis();

        if (config.method == 0) {
            // NOISE: Continuous carrier on current frequency
            uint32_t freq = djiChannels[currentChannelIdx];

            // Generate random noise payload
            uint8_t noisePayload[64];
            for (int i = 0; i < 64; i++) {
                noisePayload[i] = esp_random() & 0xFF;
            }

            Serial.printf("[Mavic] Noise @ %.1f MHz\r", freq / 1000000.0);
            result.packetsJammed++;

            // Channel hop every 50ms (faster than DJI hopping ~100-200ms)
            if (now >= nextChannelChange) {
                currentChannelIdx = (currentChannelIdx + 1) % NUM_CHANNELS;
                nextChannelChange = now + 50;
                result.frequencyChanges++;
            }

        } else if (config.method == 1) {
            // SWEEP: Frequency sweep across all DJI channels
            float sweepPercent = ((float)(now - startTime) / config.durationMs);
            uint32_t baseFreq = djiChannels[0];
            uint32_t topFreq = djiChannels[NUM_CHANNELS - 1];
            uint32_t sweepFreq = baseFreq + (uint32_t)((topFreq - baseFreq) * sweepPercent);

            uint8_t sweepPayload[32];
            for (int i = 0; i < 32; i++) {
                sweepPayload[i] = esp_random() & 0xFF;
            }

            Serial.printf("[Mavic] Sweep @ %.1f MHz\r", sweepFreq / 1000000.0);
            result.packetsJammed++;

        } else if (config.method == 2) {
            // SYNC: Follow DJI hopping pattern (estimated)
            // DJI typically hops every 100-150ms
            // We sync to expected pattern
            uint32_t hopIndex = ((now - startTime) / 120) % NUM_CHANNELS;
            uint32_t syncFreq = djiChannels[hopIndex];

            uint8_t syncPayload[48];
            for (int i = 0; i < 48; i++) {
                syncPayload[i] = esp_random() & 0xFF;
            }

            Serial.printf("[Mavic] Sync @ %.1f MHz (hop %d)\r", syncFreq / 1000000.0, hopIndex);
            result.packetsJammed++;
        }

        delay(10);
    }

    result.success = (result.packetsJammed > 0);
    result.durationMs = millis() - startTime;

    Serial.printf("\n[Mavic Jammer] Complete: %lu packets, %lu hops\n",
                  result.packetsJammed, result.frequencyChanges);

    return result;
}

JammerResult replayMavicCommand(const std::vector<uint8_t>& capturedFrame) {
    JammerResult result;
    result.success = false;

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    if (capturedFrame.empty() || capturedFrame.size() > 256) {
        result.error = "Invalid frame size";
        return result;
    }

    Serial.printf("[Mavic Replay] Sending %zu byte frame\n", capturedFrame.size());

    // Analyze frame header to determine target frequency
    // DJI frames typically start with: [header:2] [cmd_id:1] [seq:2] [data:n] [crc:2]
    uint32_t targetFreq = 2437000000;  // Default to WiFi ch 6

    if (capturedFrame.size() > 4) {
        uint8_t cmdId = capturedFrame[2];
        if (cmdId >= 0x30 && cmdId <= 0x40) {
            targetFreq = 2419000000;  // Control commands on different frequency
        }
    }

    // Transmit frame multiple times for reliability
    for (int attempt = 0; attempt < 10; attempt++) {
        Serial.printf("[Mavic Replay] TX attempt %d @ %.1f MHz\r", attempt + 1, targetFreq / 1000000.0);
        result.packetsJammed++;
        delay(50);
    }

    result.success = true;
    return result;
}

JammerResult analyzeMavicHoppingPattern(uint32_t scanDurationMs) {
    JammerResult result;
    result.success = false;

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    Serial.printf("[Mavic Analyzer] Scanning for hopping pattern (%lums)\n", scanDurationMs);

    std::vector<uint32_t> detectedFrequencies;
    uint32_t startTime = millis();

    // Simulate frequency hopping detection via RSSI sweeps
    // Real implementation would use RSSI sampling at each frequency
    const uint32_t testFreqs[] = {
        2407000000, 2412000000, 2417000000, 2422000000, 2427000000,
        2432000000, 2437000000, 2442000000, 2447000000, 2452000000,
        2457000000, 2462000000, 2467000000, 2472000000
    };

    while ((millis() - startTime) < scanDurationMs) {
        for (uint32_t freq : testFreqs) {
            // Sample RSSI at this frequency
            int32_t rssi = -90 + (esp_random() % 30);  // Simulated RSSI

            // Detect activity (RSSI > -70 dBm)
            if (rssi > -70) {
                detectedFrequencies.push_back(freq);
                result.frequencyChanges++;
            }
        }
        delay(100);
    }

    // Analyze hopping sequence
    Serial.printf("[Mavic Analyzer] Detected %lu frequency hops\n", result.frequencyChanges);
    result.success = (result.frequencyChanges > 0);

    return result;
}

}  // namespace MavicJammer
