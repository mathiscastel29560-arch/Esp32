#pragma once
#include <Arduino.h>
#include <vector>

// Passive Drone Tracking: Detect and localize drones by 2.4GHz signal strength (RSSI)
// No transmission - fully passive listening
// Can triangulate position with 3+ sensors
namespace DroneTracker {

struct DroneSignal {
    uint32_t timestamp;       // millis() when detected
    int16_t rssi;             // Signal strength (-100 to -30 dBm)
    uint16_t frequency;       // 2400-2500 MHz
    uint32_t detectionCount;  // How many times this drone detected
    float estimatedDistance;  // Rough distance estimate (meters)
};

struct ScanResult {
    uint32_t activeDroneCount;     // Number of distinct drone signals
    int16_t strongestSignal;       // RSSI of strongest signal
    float averageDistance;         // Avg distance to detected drones
    uint32_t scanDurationMs;
    std::vector<DroneSignal> signals;
};

// Scan for 2.4GHz drone signals (hopping across channels)
// Returns drone detections with RSSI strength
ScanResult scanForDrones(uint32_t durationMs = 5000);

// Continuous monitoring mode (blocks until stopped)
void monitorDrones(uint32_t checkIntervalMs = 1000);

// Stop the monitoring loop
void stopMonitoring();

// Estimate distance from RSSI (very rough, empirical)
// Path Loss Model: d = 10^((txPower - rssi) / 20)
float estimateDistance(int16_t rssi, int16_t txPowerDbm = 0);

// Log drone detections to file for triangulation analysis
void logDetectionsToFile(const String &filename, const ScanResult &result);

// Triangulate drone position from 3 sensor readings
// Returns lat/lon if possible, else {0,0}
struct Position {
    float latitude;
    float longitude;
};

Position triangulateDrone(const DroneSignal &sig1, const DroneSignal &sig2, const DroneSignal &sig3,
                         const Position &sensor1Pos, const Position &sensor2Pos, const Position &sensor3Pos);

}  // namespace DroneTracker
