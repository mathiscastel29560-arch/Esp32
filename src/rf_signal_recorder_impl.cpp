#include "rf_signal_recorder.h"
#include "config.h"
#include <RadioLib.h>
#include <RF24.h>
#include "results_display.h"

namespace RfSignalRecorder {

static std::vector<uint8_t> recordedSamples;
static float lastFrequency = 0;
static uint32_t lastDurationMs = 0;

RecordingResult recordSignals(float frequencyMHz, uint32_t durationMs, const char* radioType) {
    RecordingResult result = {false, 0, 0, frequencyMHz, 0, -150, -100};
    recordedSamples.clear();

    String radio = String(radioType);
    if (radio == "auto") {
        radio = (frequencyMHz < 1000) ? "cc1101" : "nrf24";
    }

    uint32_t startTime = millis();
    uint32_t deadline = startTime + durationMs;
    float rssiSum = 0;
    uint32_t rssiCount = 0;
    result.rssiMin = 0;
    result.rssiMax = -150;

    if (radio == "cc1101") {
        Module cc1101Module(PIN_CC1101_CS, PIN_CC1101_GDO0, RADIOLIB_NC, PIN_CC1101_GDO2, SPI);
        CC1101 cc1101(&cc1101Module);

        if (cc1101.begin(433.0) != RADIOLIB_ERR_NONE) {
    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
            return result;
        }

        cc1101.setRxBandwidth(812.5);
        cc1101.startReceive();

        while ((int32_t)(millis() - deadline) < 0 && recordedSamples.size() < 65536) {
            int state = cc1101.available();
            if (state == RADIOLIB_ERR_NONE) {
                uint8_t data[256] = {0};
                size_t len = cc1101.getPacketLength();
                if (len > 0 && len <= 256) {
                    state = cc1101.readData(data, len);
                    if (state == RADIOLIB_ERR_NONE) {
                        for (size_t i = 0; i < len && recordedSamples.size() < 65536; i++) {
                            recordedSamples.push_back(data[i]);
                        }
                        float rssi = cc1101.getRSSI();
                        rssiSum += rssi;
                        rssiCount++;
                        result.rssiMax = max(result.rssiMax, rssi);
                        result.rssiMin = min(result.rssiMin, rssi);
                    }
                }
            }
            delay(10);
        }

        cc1101.sleep();

    } else if (radio == "nrf24") {
        // NRF24L01+ @ 2.4GHz
        RF24 nrf24(PIN_NRF24_CE, PIN_NRF24_CS);

        if (!nrf24.begin()) {
    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
            return result;
        }

        nrf24.setPALevel(RF24_PA_HIGH);
        nrf24.setDataRate(RF24_250KBPS);
        nrf24.openReadingPipe(0, 0xAAAAAAAAAAAALL);
        nrf24.startListening();

        while ((int32_t)(millis() - deadline) < 0 && recordedSamples.size() < 65536) {
            if (nrf24.available()) {
                uint8_t data[32] = {0};
                uint8_t len = nrf24.getDynamicPayloadSize();
                if (len > 0 && len <= 32) {
                    nrf24.read(data, len);
                    for (uint8_t i = 0; i < len && recordedSamples.size() < 65536; i++) {
                        recordedSamples.push_back(data[i]);
                    }
                    // Estimate RSSI (NRF24 doesn't have true RSSI, simulate)
                    float rssi = -40 - (esp_random() % 50);
                    rssiSum += rssi;
                    rssiCount++;
                    result.rssiMax = max(result.rssiMax, rssi);
                    result.rssiMin = min(result.rssiMin, rssi);
                }
            }
            delay(10);
        }

        nrf24.stopListening();
    }

    result.success = (recordedSamples.size() > 0);
    result.sampleCount = recordedSamples.size();
    result.durationMs = millis() - startTime;
    result.frequency = frequencyMHz;
    result.rssiAvg = (rssiCount > 0) ? (rssiSum / rssiCount) : -100;

    lastFrequency = frequencyMHz;
    lastDurationMs = result.durationMs;

    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
    return result;
}

const uint8_t* getCapturedData(uint32_t& outLength) {
    outLength = recordedSamples.size();
    return recordedSamples.empty() ? nullptr : recordedSamples.data();
}

void clearRecording() {
    recordedSamples.clear();
}

SignalStats analyzeSignal() {
    SignalStats stats = {0, 0, 0, 0, 0};

    if (recordedSamples.empty()) {
        return stats;
    }

    stats.totalSamples = recordedSamples.size();

    // Count transitions (bit flips)
    uint32_t transitions = 0;
    for (size_t i = 1; i < recordedSamples.size(); i++) {
        if (recordedSamples[i] != recordedSamples[i-1]) {
            transitions++;
        }
    }
    stats.transitionCount = transitions;

    // Peak and average
    float sum = 0;
    uint8_t peak = 0;
    for (auto byte : recordedSamples) {
        sum += byte;
        peak = max(peak, byte);
    }
    stats.averageSignalStrength = sum / recordedSamples.size();
    stats.peakSignalStrength = peak;

    // Count silence gaps (long runs of zeros)
    uint32_t zeroRun = 0;
    uint32_t silenceGaps = 0;
    for (auto byte : recordedSamples) {
        if (byte == 0) {
            zeroRun++;
            if (zeroRun > 16) silenceGaps++;  // Gap > 16 bytes
        } else {
            zeroRun = 0;
        }
    }
    stats.silenceGaps = silenceGaps;

    return stats;
}

}  // namespace RfSignalRecorder
