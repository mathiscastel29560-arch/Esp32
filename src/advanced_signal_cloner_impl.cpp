#include "advanced_signal_cloner.h"
#include "rf_signal_recorder.h"
#include "signal_decoder.h"
#include "config.h"
#include <RadioLib.h>
#include <RF24.h>
#include "tool_output_helper.h"
#include "result_renderers.h"
#include "audit_log.h"
#include "tool_result_persistence.h"

namespace AdvancedSignalCloner {

static uint32_t lastTransmittedBytes = 0;
static uint32_t totalCloneAttempts = 0;
static uint32_t successfulClones = 0;
static float totalTransmitTime = 0;

CloneResult cloneSignal(const CloneParams& params) {
    CloneResult result = {false, 0, 0, 0, "NONE"};

    uint32_t dataLen = 0;
    const uint8_t* data = RfSignalRecorder::getCapturedData(dataLen);

    if (!data || dataLen == 0) {
        return result;
    }

    if (!validateSignal()) {
        return result;
    }

    uint32_t startTime = millis();
    uint32_t totalBytes = 0;
    uint32_t completedReps = 0;

    // Transmit on appropriate radio based on frequency
    bool useCC1101 = (params.frequency < 1000);  // Sub-GHz
    bool useNRF24 = (params.frequency >= 2000 && params.frequency < 3000);  // 2.4GHz

    if (useCC1101) {
        // Use CC1101 module (Sub-GHz)
        Module cc1101Module(PIN_CC1101_CS, PIN_CC1101_GDO0, RADIOLIB_NC, PIN_CC1101_GDO2, SPI);
        CC1101 cc1101(&cc1101Module);

        if (cc1101.begin(params.frequency) != RADIOLIB_ERR_NONE) {
            return result;
        }

        cc1101.setOutputPower(params.powerLevel);

        uint8_t* mutableData = new uint8_t[dataLen];
        memcpy(mutableData, data, dataLen);

        for (uint32_t rep = 0; rep < params.repeatCount; rep++) {
            int state = cc1101.transmit(mutableData, dataLen);
            if (state == RADIOLIB_ERR_NONE) {
                totalBytes += dataLen;
                completedReps++;
            }
            if (rep < params.repeatCount - 1) {
                delay(params.delayBetweenRepsMs);
            }
        }

        cc1101.sleep();
        delete[] mutableData;
        result.radioUsed = "CC1101";

    } else if (useNRF24) {
        // Use NRF24 module (2.4GHz)
        RF24 nrf24(PIN_NRF24_CE, PIN_NRF24_CS);

        if (!nrf24.begin()) {
            return result;
        }

        nrf24.setPALevel(RF24_PA_HIGH);
        nrf24.setDataRate(RF24_250KBPS);
        nrf24.openWritingPipe(0xAAAAAAAAAAAALL);
        nrf24.stopListening();

        for (uint32_t rep = 0; rep < params.repeatCount; rep++) {
            if (nrf24.write(data, dataLen)) {
                totalBytes += dataLen;
                completedReps++;
            }
            if (rep < params.repeatCount - 1) {
                delay(params.delayBetweenRepsMs);
            }
        }

        nrf24.stopListening();
        result.radioUsed = "NRF24";
    }

    result.success = (completedReps > 0);
    result.transmittedBytes = totalBytes;
    result.repetitionsCompleted = completedReps;
    result.durationMs = millis() - startTime;

    lastTransmittedBytes = totalBytes;
    totalCloneAttempts++;
    if (result.success) {
        successfulClones++;
        totalTransmitTime += result.durationMs;
    }

    return result;
}

CloneResult quickReplay(uint32_t repeatCount) {
    CloneParams params;
    params.frequency = 433.0;
    params.bitrate = 2400;
    params.powerLevel = 20;
    params.repeatCount = repeatCount;
    params.delayBetweenRepsMs = 100;
    params.modulationType = "ASK";
    params.useTimingInfo = false;

    return cloneSignal(params);
}

CloneResult cloneAt2400MHz(uint32_t repeatCount) {
    CloneParams params;
    params.frequency = 2400.0;
    params.bitrate = 250000;
    params.powerLevel = 20;
    params.repeatCount = repeatCount;
    params.delayBetweenRepsMs = 100;
    params.modulationType = "GFSK";
    params.useTimingInfo = false;

    return cloneSignal(params);
}

CloneResult cloneWithOriginalTiming(uint32_t repeatCount) {
    CloneParams params;
    params.frequency = 433.0;
    params.bitrate = 2400;
    params.powerLevel = 20;
    params.repeatCount = repeatCount;
    params.delayBetweenRepsMs = 50;
    params.modulationType = "ASK";
    params.useTimingInfo = true;

    return cloneSignal(params);
}

bool validateSignal() {
    uint32_t dataLen = 0;
    const uint8_t* data = RfSignalRecorder::getCapturedData(dataLen);

    if (!data || dataLen == 0) {
        return false;
    }

    // Check if signal has reasonable length
    if (dataLen < 4 || dataLen > 65536) {
        return false;
    }

    // Check for minimal content (not all zeros or all ones)
    uint32_t zeros = 0;
    uint32_t ones = 0;
    for (uint32_t i = 0; i < min((uint32_t)1024, dataLen); i++) {
        if (data[i] == 0) zeros++;
        else if (data[i] == 0xFF) ones++;
    }

    // Allow at least 20% variation
    uint32_t sampleSize = min((uint32_t)1024, dataLen);
    if (zeros > sampleSize * 0.8 || ones > sampleSize * 0.8) {
        return false;
    }

    return true;
}

CloneStats getCloneStats() {
    CloneStats stats;
    stats.lastTransmittedBytes = lastTransmittedBytes;
    stats.totalCloneAttempts = totalCloneAttempts;
    stats.successfulClones = successfulClones;
    stats.averageTransmitTime = (successfulClones > 0) ? (totalTransmitTime / successfulClones) : 0;
    return stats;
}

}  // namespace AdvancedSignalCloner
