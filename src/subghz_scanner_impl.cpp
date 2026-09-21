#include "subghz_scanner.h"
#include "config.h"
#include <RadioLib.h>

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
    ScanResult result{0, -127, 0.0f, durationMs, {}};
    
    Serial.println("\n=== Sub-GHz Band Scanner (433 MHz) ===");
    Serial.println("Scanning 433.05 - 434.79 MHz");
    Serial.println("Duration: " + String(durationMs) + "ms");
    
    uint32_t startTime = millis();
    
    // Scan 433 MHz band in 100kHz steps
    for (float freq = 433.05f; freq <= 434.79f; freq += 0.1f) {
        if (millis() - startTime > durationMs) break;
        
        int8_t rssi = scanFrequency(freq);
        
        if (rssi > -90) {  // Signal detected
            String sigType = (rssi > -70) ? "Strong" : (rssi > -80) ? "Medium" : "Weak";
            SignalDetection det{freq, rssi, millis(), sigType};
            result.detections.push_back(det);
            result.detectionCount++;
            
            if (rssi > result.strongestSignal) {
                result.strongestSignal = rssi;
                result.busyFrequency = freq;
            }
            
            Serial.println("  [" + String(freq, 2) + " MHz] RSSI: " + String(rssi) + 
                         "dBm (" + sigType + ")");
        }
        
        delay(50);
    }
    
    Serial.println("\n=== Scan Complete ===");
    Serial.println("Signals found: " + String(result.detectionCount));
    Serial.println("Strongest: " + String(result.strongestSignal) + "dBm @ " + 
                  String(result.busyFrequency, 2) + " MHz");
    
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
