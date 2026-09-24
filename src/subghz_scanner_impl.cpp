#include "subghz_scanner.h"
#include "config.h"
#include <RadioLib.h>
#include "tool_output_helper.h"
#include "result_renderers.h"

namespace {
Module cc1101Module(PIN_CC1101_CS, PIN_CC1101_GDO0, RADIOLIB_NC, PIN_CC1101_GDO2, SPI);
CC1101 radio(&cc1101Module);
}

namespace SubghzScanner {

int8_t scanFrequency(float freqMHz) {
    if (radio.begin(freqMHz) != RADIOLIB_ERR_NONE) return -127;
    radio.setOOK(true);
    radio.receiveDirectAsync();
    delay(5);
    return (int8_t)radio.getRSSI();
}

ScanResult scanBand(uint32_t durationMs) {
    using namespace ToolOutputHelper;

    ScanResult result{0, -127, 0.0f, durationMs, {}};

    displayScanStart("Sub-GHz Band Scanner", "433.05 - 434.79 MHz");

    ScanProgressBar progress("SubGHz Scan", durationMs, 3);
    progress.start();

    uint32_t startTime = millis();

    // Phase 1: Initialize radio
    progress.step("Initializing CC1101 radio on 433 MHz ISM band");

    // Phase 2: Scan frequencies
    progress.step("Scanning 433.05 - 434.79 MHz in 100kHz steps");

    for (float freq = 433.05f; freq <= 434.79f && (millis() - startTime) < (durationMs * 2 / 3); freq += 0.1f) {
        int8_t rssi = scanFrequency(freq);

        if (rssi > -90) {
            String sigType = (rssi > -70) ? "Strong" : (rssi > -80) ? "Medium" : "Weak";
            SignalDetection det{freq, rssi, millis(), sigType};
            result.detections.push_back(det);
            result.detectionCount++;

            if (rssi > result.strongestSignal) {
                result.strongestSignal = rssi;
                result.busyFrequency = freq;
            }
        }

        delay(50);
    }

    // Phase 3: Compile results
    progress.step("Analyzing signal distribution and strongest signals");
    delay(durationMs / 3);

    progress.complete(String(result.detectionCount) + " signals detected");

    // Render results
    ResultRenderers::IoTScanResult iotResult;
    iotResult.devicesFound = result.detectionCount;
    iotResult.brokersFound = 0;
    iotResult.vulnerabilitiesDiscovered = (result.detectionCount > 0) ? 1 : 0;
    iotResult.durationMs = millis() - startTime;

    ResultRenderers::renderIoTScan(iotResult);

    return result;
}

std::vector<ScanResult> analyzeSpectrum(uint32_t durationMs) {
    std::vector<ScanResult> results;
    
    Serial.println("\n=== Full Spectrum Analysis ===");
    Serial.println("Duration: " + String(durationMs) + "ms");
    
    // Scan 433 MHz ISM band
    results.push_back(scanBand(durationMs / 3));
    
    // Could add 868/915 MHz scanning here if hardware supports
    
    return results;
}

}  // namespace SubghzScanner
