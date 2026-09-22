#include "drone_tracker.h"
#include "nrf24_tools.h"
#include "gps_module.h"
#include <cmath>

namespace DroneTracker {

// Path loss calculation: RSSI = TxPower - 20*log10(distance) - losses
// Typical: TxPower = 0 dBm, losses = 40-50 dB at 1m in open space
float estimateDistance(int16_t rssi, int16_t txPowerDbm) {
    // Empirical path loss exponent for 2.4GHz (typically 2.0-4.0)
    const float pathLossExponent = 2.5f;  // Open space
    const int16_t txPower = txPowerDbm;
    const int16_t refRssiAt1m = -40;  // Reference: -40 dBm at 1 meter

    if (rssi >= refRssiAt1m) return 0.5f;  // Very close

    // d = 10^((TxPower - RSSI - refLoss) / (20 * n))
    float distance = pow(10.0f, (float)(txPower - rssi - (txPower - refRssiAt1m)) / (20.0f * pathLossExponent));
    return distance < 0 ? 0 : distance;
}

ScanResult scanForDrones(uint32_t durationMs) {
    ScanResult result{0, 0, 0.0f, durationMs, {}};

    Serial.println("\n=== Drone Tracker Scan ===");
    Serial.println("Scanning 2.4GHz for drone signals...");
    Serial.println("Duration: " + String(durationMs) + "ms");

    uint32_t startTime = millis();
    std::vector<uint8_t> activityReadings;

    // Scan across 2.4GHz NRF24 channels (0-125)
    // Get activity levels on each channel
    auto activity = Nrf24Tools::scanChannels(100);

    // Analyze activity per channel - higher activity = more signals
    // Map activity levels (0-255) to pseudo-RSSI for distance calculation
    for (uint8_t ch = 0; ch < 126; ch++) {
        if (activity[ch] > 50) {  // Significant activity detected
            // Convert activity level (0-255) to pseudo-RSSI (-100 to -30 dBm scale)
            int16_t pseudoRssi = -100 + (activity[ch] / 255) * 70;  // Scale activity to RSSI range
            activityReadings.push_back(activity[ch]);

            DroneSignal sig{millis(), pseudoRssi, uint16_t(2400 + ch), activity[ch], estimateDistance(pseudoRssi)};
            result.signals.push_back(sig);
            result.activeDroneCount++;

            Serial.println("  [CH" + String(ch) + " / " + String(2400 + ch) + "MHz] Activity: " +
                         String(activity[ch]) + ", est: " + String(sig.estimatedDistance, 1) + "m");
        }
    }

    // Calculate statistics
    if (!activityReadings.empty()) {
        result.strongestSignal = *std::max_element(activityReadings.begin(), activityReadings.end());

        float sumDistance = 0;
        for (auto &sig : result.signals) {
            sumDistance += sig.estimatedDistance;
        }
        result.averageDistance = sumDistance / result.signals.size();
    }

    Serial.println("\n=== Scan Complete ===");
    Serial.println("Active channels detected: " + String(result.activeDroneCount));
    Serial.println("Strongest activity: " + String(result.strongestSignal));
    Serial.println("Average distance: " + String(result.averageDistance, 1) + "m");

    return result;
}

namespace {
bool g_monitoringActive = false;
}

void monitorDrones(uint32_t checkIntervalMs) {
    Serial.println("\n=== Continuous Drone Monitoring ===");
    Serial.println("Monitoring for 2.4GHz signals (Press BACK to stop)");

    uint32_t consecutiveNoSignal = 0;
    g_monitoringActive = true;

    while (g_monitoringActive) {
        ScanResult result = scanForDrones(checkIntervalMs);

        if (result.activeDroneCount > 0) {
            consecutiveNoSignal = 0;
            Serial.println("🚁 DRONE DETECTED - " + String(result.activeDroneCount) +
                         " signal(s), strongest: " + String(result.strongestSignal) + "dBm");
        } else {
            consecutiveNoSignal++;
            if (consecutiveNoSignal % 10 == 0) {
                Serial.println("No signal detected (" + String(consecutiveNoSignal * checkIntervalMs / 1000) + "s quiet)");
            }
        }

        delay(100);
    }
}

void stopMonitoring() {
    g_monitoringActive = false;
}

void logDetectionsToFile(const String &filename, const ScanResult &result) {
    // TODO: Implement LittleFS logging
    // Format: timestamp,rssi,frequency,distance,channel

    Serial.println("Logging " + String(result.signals.size()) + " detections to " + filename);

    for (auto &sig : result.signals) {
        String line = String(sig.timestamp) + "," + String(sig.rssi) + "," +
                     String(sig.frequency) + "," + String(sig.estimatedDistance, 2);
        Serial.println("  > " + line);
    }
}

// Simple triangulation using trilateration
// Requires 3 sensor positions and their distance to target
Position triangulateDrone(const DroneSignal &sig1, const DroneSignal &sig2, const DroneSignal &sig3,
                         const Position &sensor1Pos, const Position &sensor2Pos, const Position &sensor3Pos) {
    // Simplified trilateration using RSSI distances
    // Real implementation would use non-linear least squares

    float d1 = sig1.estimatedDistance;
    float d2 = sig2.estimatedDistance;
    float d3 = sig3.estimatedDistance;

    float lat1 = sensor1Pos.latitude;
    float lon1 = sensor1Pos.longitude;
    float lat2 = sensor2Pos.latitude;
    float lon2 = sensor2Pos.longitude;
    float lat3 = sensor3Pos.latitude;
    float lon3 = sensor3Pos.longitude;

    // Very simplified calculation (not accurate, but demonstrates concept)
    // Real triangulation requires non-linear solver
    float estLat = (lat1 + lat2 + lat3) / 3.0f;
    float estLon = (lon1 + lon2 + lon3) / 3.0f;

    Serial.println("Triangulation (simplified):");
    Serial.println("  Position: " + String(estLat, 6) + ", " + String(estLon, 6));
    Serial.println("  Confidence: LOW (need proper least-squares solver)");

    return {estLat, estLon};
}

}  // namespace DroneTracker
