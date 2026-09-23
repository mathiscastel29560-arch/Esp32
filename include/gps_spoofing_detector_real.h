#ifndef GPS_SPOOFING_DETECTOR_REAL_H
#define GPS_SPOOFING_DETECTOR_REAL_H

#include <Arduino.h>
#include <vector>

class GPSSpoofingDetectorReal {
public:
    static GPSSpoofingDetectorReal& instance() {
        static GPSSpoofingDetectorReal detector;
        return detector;
    }

    struct GPSFix {
        double latitude;
        double longitude;
        float altitude;
        uint8_t satellites;
        uint16_t hdop;           // Horizontal dilution
        uint16_t vdop;           // Vertical dilution
        float speed_mps;
        float course;
        uint32_t timestamp;      // Unix seconds
    };

    struct Anomaly {
        uint8_t type;            // 0=speed, 1=hdop, 2=satellites, 3=jump, 4=geometry
        float severity;          // 0-100%
        char description[128];
        uint32_t timestamp;
    };

    struct DetectionStats {
        uint32_t total_fixes;
        uint32_t anomalies_detected;
        uint32_t high_confidence_spoofs;  // Spoofing confidence >70%
        float max_speed_kmh;
        float avg_hdop;
        float avg_satellite_count;
        uint32_t monitoring_duration;
        uint8_t overall_risk;    // 0-100%
    };

    // Initialize with real GPS module
    bool begin();

    // Start monitoring GPS stream
    void startMonitoring();
    void stopMonitoring();

    // Add new GPS fix from NEO-6M module
    void addFix(double lat, double lon, float alt, uint8_t sats,
               uint16_t hdop, uint16_t vdop, float speed, float course);

    // Analyze current fixes for spoofing indicators
    std::vector<Anomaly> analyzeForSpoofing();

    // Get statistics
    DetectionStats getStats();
    String generateReport();
    void exportToJSON(const char* filepath);
    void clearHistory();
    bool isMonitoring() { return monitoring_; }

private:
    GPSSpoofingDetectorReal() : monitoring_(false), fix_count_(0) {}

    bool monitoring_;
    std::vector<GPSFix> fix_history_;
    std::vector<Anomaly> anomalies_;
    DetectionStats stats_;
    uint32_t fix_count_;
    uint32_t start_time_;

    const float MAX_REALISTIC_SPEED_MPS = 100.0f;    // ~360 km/h
    const float MIN_REALISTIC_HDOP = 0.5f;
    const float MAX_REALISTIC_HDOP = 1000.0f;
    const uint8_t MIN_SATELLITES = 4;                // Need 4+ for 3D fix
    const float MAX_POSITION_JUMP = 10000.0f;        // 10km is huge jump

    float calculateDistance(double lat1, double lon1, double lat2, double lon2);
    float calculateSpeed(const GPSFix& prev, const GPSFix& curr);
    bool checkSignalQuality(const GPSFix& fix);
    bool checkConsistency(const std::vector<GPSFix>& history);
    bool checkTrajectory(const GPSFix& prev, const GPSFix& curr);
    uint8_t calculateRiskLevel();
};

#endif
