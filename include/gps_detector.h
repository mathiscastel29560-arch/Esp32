#pragma once
#include <Arduino.h>
#include <vector>

namespace GPSDetector {

struct GPSFix {
    double latitude;
    double longitude;
    float altitude;
    uint8_t satellites;
    uint16_t hdop;
    uint32_t timestamp;
};

struct AnomalyIndicator {
    uint8_t type;  // 0=speed, 1=hdop, 2=satellites, 3=jump, 4=geometry
    float severity;
    char description[128];
};

struct DetectionStats {
    uint32_t total_fixes;
    uint32_t anomalies_detected;
    uint32_t spoofing_confidence;  // 0-100%
    float max_speed_kmh;
    float avg_hdop;
};

// Initialize GPS spoofing detection
bool begin();

// Add GPS fix to stream for analysis
void addFix(double lat, double lon, float alt, uint8_t sats, uint16_t hdop);

// Analyze accumulated fixes for spoofing
std::vector<AnomalyIndicator> analyzeForSpoofing();

// Get statistics
DetectionStats getStats();

// Generate report
String generateReport();

// Reset data
void reset();

}  // namespace GPSDetector
