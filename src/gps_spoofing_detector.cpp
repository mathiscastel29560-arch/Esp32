#include "gps_spoofing_detector.h"
#include "audit_log.h"
#include <cmath>

#define EARTH_RADIUS_M 6371000.0
#define MAX_REALISTIC_SPEED 300.0  // m/s (1080 km/h, faster than commercial aircraft)
#define NORMAL_HDOP_RANGE 50  // Horizontal dilution of precision

void GPSSpoofingDetector::begin() {
    fix_history_.clear();
    fix_history_.reserve(200);
    anomalies_.clear();
    memset(&stats_, 0, sizeof(DetectionStats));
    Serial.println("[GPSSpoofingDetector] GPS spoofing detector initialized");
    AuditLog::instance().log(AuditEventType::TOOL_START, "GPS_Detector", "GPS spoofing detection monitor activated");
}

void GPSSpoofingDetector::addFix(double lat, double lon, float alt, uint8_t sats, uint16_t hdop) {
    if (fix_history_.size() >= 200) {
        fix_history_.erase(fix_history_.begin());
    }

    GPSFix fix = {
        .latitude = lat,
        .longitude = lon,
        .altitude = alt,
        .satellites = sats,
        .hdop = hdop,
        .timestamp = millis() / 1000
    };

    fix_history_.push_back(fix);
    fix_count_++;
    stats_.total_fixes = fix_count_;
    stats_.avg_hdop = (stats_.avg_hdop + hdop) / 2;
}

void GPSSpoofingDetector::startMonitoring() {
    monitoring_ = true;
    fix_count_ = 0;
    start_time_ = millis();
    fix_history_.clear();
    anomalies_.clear();
    Serial.println("[GPSSpoofingDetector] GPS monitoring started");
    AuditLog::instance().log(AuditEventType::TOOL_START, "GPS_Detector", "GPS spoofing monitoring session started");
}

void GPSSpoofingDetector::stopMonitoring() {
    monitoring_ = false;
    stats_.monitoring_duration = (millis() - start_time_) / 1000;

    if (!anomalies_.empty()) {
        stats_.spoofing_confidence = 40 + (anomalies_.size() * 10);
        if (stats_.spoofing_confidence > 100) stats_.spoofing_confidence = 100;
        AuditLog::instance().log(AuditEventType::TOOL_SUCCESS, "GPS_Detector", "Potential GPS spoofing detected");
    }

    Serial.printf("[GPSSpoofingDetector] Monitoring stopped. Duration: %u seconds, Anomalies: %u\n",
                 stats_.monitoring_duration, stats_.anomalies_detected);
}

std::vector<GPSSpoofingDetector::AnomalyIndicator> GPSSpoofingDetector::analyzePattern() {
    anomalies_.clear();

    if (fix_history_.size() < 2) return anomalies_;

    // Check for impossible speeds
    for (size_t i = 1; i < fix_history_.size(); i++) {
        float speed = calculateSpeed(fix_history_[i - 1], fix_history_[i]);

        if (speed > MAX_REALISTIC_SPEED) {
            AnomalyIndicator anom = {
                .type = 0,  // Speed anomaly
                .severity = static_cast<float>((speed / (MAX_REALISTIC_SPEED * 2)) * 100.0f),
                .timestamp = fix_history_[i].timestamp
            };
            anom.severity = (anom.severity > 100.0f) ? 100.0f : anom.severity;
            snprintf(anom.description, sizeof(anom.description),
                    "Impossible speed: %.1f m/s", speed);
            anomalies_.push_back(anom);
            stats_.max_speed_mps = (speed > stats_.max_speed_mps) ? speed : stats_.max_speed_mps;
        }
    }

    // Check for signal consistency
    for (const auto& fix : fix_history_) {
        if (!checkSignalConsistency(fix.hdop)) {
            AnomalyIndicator anom = {
                .type = 2,  // Signal anomaly
                .severity = (fix.hdop > (NORMAL_HDOP_RANGE * 2)) ? 75.0f : 40.0f,
                .timestamp = fix.timestamp
            };
            snprintf(anom.description, sizeof(anom.description),
                    "Signal degradation: HDOP=%u", fix.hdop);
            anomalies_.push_back(anom);
        }
    }

    // Check geometric validity
    if (!checkGeometricValidity(fix_history_)) {
        AnomalyIndicator anom = {
            .type = 3,  // Geometric anomaly
            .severity = 60.0f,
            .timestamp = millis() / 1000
        };
        snprintf(anom.description, sizeof(anom.description),
                "Geometric inconsistency detected in trajectory");
        anomalies_.push_back(anom);
    }

    stats_.anomalies_detected = anomalies_.size();
    return anomalies_;
}

GPSSpoofingDetector::DetectionStats GPSSpoofingDetector::getStats() {
    stats_.monitoring_duration = (millis() - start_time_) / 1000;
    return stats_;
}

String GPSSpoofingDetector::generateReport() {
    String report = "\n╔════════════════════════════════════════════╗\n";
    report += "║      GPS SPOOFING DETECTION REPORT         ║\n";
    report += "╚════════════════════════════════════════════╝\n\n";

    report += String("[MONITORING STATUS]\n");
    report += String("  Status:           ") + String(monitoring_ ? "ACTIVE" : "INACTIVE") + "\n";
    report += String("  Total Fixes:      ") + String(stats_.total_fixes) + "\n";
    report += String("  Duration:         ") + String(stats_.monitoring_duration) + " seconds\n";
    report += String("  Anomalies Found:  ") + String(stats_.anomalies_detected) + "\n\n";

    report += String("[SPOOFING ANALYSIS]\n");
    report += String("  Spoofing Risk:    ");
    if (stats_.spoofing_confidence >= 70) {
        report += String("⚠️  HIGH (") + String(stats_.spoofing_confidence) + "%)\n";
    } else if (stats_.spoofing_confidence >= 40) {
        report += String("⚠️  MEDIUM (") + String(stats_.spoofing_confidence) + "%)\n";
    } else {
        report += String("✓ LOW (") + String(stats_.spoofing_confidence) + "%)\n";
    }
    report += String("  Max Speed:        ") + String(stats_.max_speed_mps, 1) + " m/s\n";
    report += String("  Avg HDOP:         ") + String(stats_.avg_hdop, 1) + "\n\n";

    if (!anomalies_.empty()) {
        report += "[DETECTED ANOMALIES]\n";
        for (size_t i = 0; i < anomalies_.size() && i < 10; i++) {
            report += String("  ") + anomalies_[i].description + "\n";
            report += String("    Severity: ") + String((int)anomalies_[i].severity) + "%\n";
        }
        report += "\n";
    }

    report += "════════════════════════════════════════════\n";
    return report;
}

void GPSSpoofingDetector::exportToJSON(const char* filepath) {
    if (!LittleFS.begin()) return;

    File f = LittleFS.open(filepath, FILE_WRITE);
    if (!f) {
        LittleFS.end();
        return;
    }

    f.print("{\"anomalies\":[");
    for (size_t i = 0; i < anomalies_.size(); i++) {
        if (i > 0) f.print(",");
        f.printf("{\"type\":%u,\"severity\":%.1f,\"description\":\"%s\",\"timestamp\":%u}",
                anomalies_[i].type, anomalies_[i].severity,
                anomalies_[i].description, anomalies_[i].timestamp);
    }
    f.printf("],\"spoofing_confidence\":%u,\"total_fixes\":%u}",
            stats_.spoofing_confidence, stats_.total_fixes);

    f.close();
    LittleFS.end();

    Serial.printf("[GPSSpoofingDetector] Analysis exported to %s\n", filepath);
    AuditLog::instance().log(AuditEventType::TOOL_SUCCESS, "GPS_Detector", "GPS spoofing analysis exported");
}

void GPSSpoofingDetector::clearHistory() {
    fix_history_.clear();
    anomalies_.clear();
    fix_count_ = 0;
    stats_.spoofing_confidence = 0;
    Serial.println("[GPSSpoofingDetector] History cleared");
}

float GPSSpoofingDetector::calculateDistance(double lat1, double lon1, double lat2, double lon2) {
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;

    double a = sin(dLat / 2.0) * sin(dLat / 2.0) +
              cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) *
              sin(dLon / 2.0) * sin(dLon / 2.0);

    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
    return EARTH_RADIUS_M * c;
}

float GPSSpoofingDetector::calculateSpeed(const GPSFix& prev, const GPSFix& curr) {
    float distance = calculateDistance(prev.latitude, prev.longitude,
                                      curr.latitude, curr.longitude);
    uint32_t time_diff = curr.timestamp - prev.timestamp;

    if (time_diff == 0) return 0.0f;
    return distance / time_diff;
}

bool GPSSpoofingDetector::checkSignalConsistency(uint16_t hdop) {
    // HDOP > 100 indicates poor signal quality
    return hdop <= (NORMAL_HDOP_RANGE * 2);
}

bool GPSSpoofingDetector::checkGeometricValidity(const std::vector<GPSFix>& history) {
    if (history.size() < 3) return true;

    // Check if last 3 points form a reasonable triangle (not too small, not degenerate)
    size_t sz = history.size();
    float d1 = calculateDistance(history[sz-3].latitude, history[sz-3].longitude,
                                 history[sz-2].latitude, history[sz-2].longitude);
    float d2 = calculateDistance(history[sz-2].latitude, history[sz-2].longitude,
                                 history[sz-1].latitude, history[sz-1].longitude);
    float d3 = calculateDistance(history[sz-1].latitude, history[sz-1].longitude,
                                 history[sz-3].latitude, history[sz-3].longitude);

    // Triangle inequality: sum of any two sides > third side
    return (d1 + d2 > d3) && (d2 + d3 > d1) && (d3 + d1 > d2);
}
