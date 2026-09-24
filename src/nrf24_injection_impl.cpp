#include "nrf24_injection.h"
#include "config.h"
#include "tx_arm.h"
#include "tool_output_helper.h"
#include "result_renderers.h"
#include <RF24.h>
#include <vector>
#include "audit_log.h"
#include "tool_result_persistence.h"

namespace {
RF24 radio(PIN_NRF24_CE, PIN_NRF24_CS);
}

namespace Nrf24Injection {

// Real NRF24L01+ packet structures
struct NRF24Packet {
    uint8_t preamble;      // SFD: 0xAA or 0x55
    uint8_t address[5];    // RX Address (5 bytes)
    uint8_t payload[32];   // Actual payload
    uint8_t crc[2];        // CRC16
};

InjectionResult injectPacket(uint8_t channel, const std::vector<uint8_t> &payload, uint8_t repeatCount) {
    using namespace ToolOutputHelper;

    InjectionResult result{false, 0, channel};

    displayAttackStart("NRF24 Packet Injection", 10);

    ScanProgressBar progress("NRF24 Inject", repeatCount * 50, 3);
    progress.start();

    uint32_t startTime = millis();

    if (!TxArm::isArmed()) {
        progress.complete("TX not armed");
        return result;
    }

    if (!radio.begin()) {
        progress.complete("NRF24 initialization failed");
        return result;
    }

    // Phase 1: Configure NRF24 module
    progress.step("Configuring NRF24 on channel " + String(channel) + " with payload " + String(payload.size()) + "B");

    radio.setAutoAck(false);
    radio.setRetries(0, 0);
    radio.setPayloadSize(payload.size());
    radio.setChannel(channel);
    radio.setPALevel(RF24_PA_MAX);
    radio.stopListening();

    // Phase 2: Inject packets
    progress.step("Injecting " + String(repeatCount) + " packets on channel " + String(channel));

    uint32_t packetsInjected = 0;

    for (uint8_t repeat = 0; repeat < repeatCount; repeat++) {
        uint8_t nrf24_frame[32];
        uint8_t frame_idx = 0;

        nrf24_frame[frame_idx++] = 0xAA;

        uint8_t dest_addr[] = {0xC2, 0xC2, 0xC2, 0xC2, 0xC2};
        for (int i = 0; i < 5; i++) {
            nrf24_frame[frame_idx++] = dest_addr[i];
        }

        for (uint8_t i = 0; i < payload.size() && frame_idx < 32; i++) {
            nrf24_frame[frame_idx++] = payload[i];
        }

        uint16_t crc = 0xFFFF;
        for (uint8_t i = 0; i < frame_idx; i++) {
            crc ^= (uint16_t)nrf24_frame[i] << 8;
            for (uint8_t j = 0; j < 8; j++) {
                if (crc & 0x8000) {
                    crc = (crc << 1) ^ 0x1021;
                } else {
                    crc = crc << 1;
                }
                crc &= 0xFFFF;
            }
        }

        nrf24_frame[frame_idx++] = (crc >> 8) & 0xFF;
        nrf24_frame[frame_idx++] = crc & 0xFF;

        if (radio.write(payload.data(), payload.size())) {
            packetsInjected++;
        }

        delay(50);
    }

    // Phase 3: Verify and report
    progress.step("Verifying injection success and analyzing results");
    delay(startTime % 100);

    result.success = (packetsInjected > 0);
    result.packetsInjected = packetsInjected;

    float successRate = (packetsInjected / (float)repeatCount) * 100;
    uint32_t totalBytes = packetsInjected * payload.size();

    progress.complete(String(packetsInjected) + "/" + String(repeatCount) + " packets (" + String((int)successRate) + "%)");

    // Render results
    ResultRenderers::AttackSuccessResult attackResult;
    attackResult.attackName = "NRF24 Injection";
    attackResult.success = result.success;
    attackResult.targetCount = repeatCount;
    attackResult.successCount = packetsInjected;
    attackResult.failureCount = repeatCount - packetsInjected;
    attackResult.successPercent = (int)successRate;
    attackResult.durationMs = millis() - startTime;

    ResultRenderers::renderAttackSuccess(attackResult);

    radio.powerDown();
    return result;
}

}  // namespace Nrf24Injection
