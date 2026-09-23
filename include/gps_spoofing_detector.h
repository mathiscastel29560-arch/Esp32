#ifndef GPS_SPOOFING_DETECTOR_H
#define GPS_SPOOFING_DETECTOR_H

#include <Arduino.h>
#include <vector>

class GPSSpoofingDetector {
public:
    static GPSSpoofingDetector& instance() {
        static GPSSpoofingDetector gsd;
        return gsd;
    }

    struct GPSFix {
        double latitude;
        double longitude;
        float altitude;
        uint8_t satellites;
        uint16_t hdop;
        uint32_t timestamp;
    };

    struct AnomalyIndicator {
        uint8_t type;  // 0=speed, 1=jump, 2=signal, 3=geometry
        float severity;  // 0-100%
        char description[64];
        uint32_t timestamp;
    };

    struct DetectionStats {
        uint32_t total_fixes;
        uint32_t anomalies_detected;
        uint32_t spoofing_confidence;  // 0-100%
        float max_speed_mps;
        float avg_hdop;
        uint32_t monitoring_duration;
    };

    void begin();
    void addFix(double lat, double lon, float alt, uint8_t sats, uint16_t hdop);
    void startMonitoring();
    void stopMonitoring();

    std::vector<AnomalyIndicator> analyzePattern();
    DetectionStats getStats();
    String generateReport();
    void exportToJSON(const char* filepath);
    void clearHistory();

private:
    GPSSpoofingDetector() : monitoring_(false), fix_count_(0) {}

    bool monitoring_;
    std::vector<GPSFix> fix_history_;
    std::vector<AnomalyIndicator> anomalies_;
    DetectionStats stats_;
    uint32_t fix_count_;
    uint32_t start_time_;

    float calculateDistance(double lat1, double lon1, double lat2, double lon2);
    float calculateSpeed(const GPSFix& prev, const GPSFix& curr);
    bool checkSignalConsistency(uint16_t hdop);
    bool checkGeometricValidity(const std::vector<GPSFix>& history);
};

#endif
