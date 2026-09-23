#include "gps_spoofing_detector_real.h"
#include "audit_log.h"
#include <cmath>

#define EARTH_RADIUS_M 6371000.0

bool GPSSpoofingDetectorReal::begin() {
    // GPS module (NEO-6M) is initialized via GpsModule::begin()
    // This detector just analyzes the stream
    fix_history_.clear();
    fix_history_.reserve(200);
    anomalies_.clear();
    memset(&stats_, 0, sizeof(DetectionStats));

    Serial.println("[GPSSpoofingDetector] GPS spoofing detection initialized");
    AuditLog::instance().log(AuditEventType::TOOL_START, "GPSSpoofingDetector",
                            "GPS spoofing detector initialized");
    return true;
}

void GPSSpoofingDetectorReal::startMonitoring() {
    monitoring_ = true;
    fix_count_ = 0;
    start_time_ = millis();
    fix_history_.clear();
    anomalies_.clear();

    Serial.println("[GPSSpoofingDetector] GPS monitoring started - analyzing stream");
    AuditLog::instance().log(AuditEventType::TOOL_START, "GPSSpoofingDetector",
                            "GPS stream monitoring active");
}

void GPSSpoofingDetectorReal::stopMonitoring() {
    monitoring_ = false;
    stats_.monitoring_duration = (millis() - start_time_) / 1000;
    stats_.overall_risk = calculateRiskLevel();

    Serial.printf("[GPSSpoofingDetector] Monitoring stopped. Duration: %u seconds\n",
                 stats_.monitoring_duration);

    if (stats_.high_confidence_spoofs > 0) {
        AuditLog::instance().log(AuditEventType::TOOL_SUCCESS, "GPSSpoofingDetector",
                                "GPS spoofing detected with high confidence");
    }
}

void GPSSpoofingDetectorReal::addFix(double lat, double lon, float alt, uint8_t sats,
                                     uint16_t hdop, uint16_t vdop, float speed, float course) {
    if (!monitoring_) return;
    if (fix_history_.size() >= 200) {
        fix_history_.erase(fix_history_.begin());
    }

    GPSFix fix = {
        .latitude = lat,
        .longitude = lon,
        .altitude = alt,
        .satellites = sats,
        .hdop = hdop,
        .vdop = vdop,
        .speed_mps = speed,
        .course = course,
        .timestamp = millis() / 1000
    };

    fix_history_.push_back(fix);
    fix_count_++;
    stats_.total_fixes = fix_count_;

    // Update running averages
    stats_.avg_hdop = (stats_.avg_hdop + hdop) / 2.0f;
    stats_.avg_satellite_count = (stats_.avg_satellite_count + sats) / 2.0f;
}

std::vector<GPSSpoofingDetectorReal::Anomaly> GPSSpoofingDetectorReal::analyzeForSpoofing() {
    anomalies_.clear();

    if (fix_history_.size() < 2) {
        Serial.println("[GPSSpoofingDetector] Not enough fixes for analysis");
        return anomalies_;
    }

    // ============ SPEED ANOMALY CHECK ============
    // Detect impossible speeds (faster than commercial aircraft)
    for (size_t i = 1; i < fix_history_.size(); i++) {
        float calculated_speed = calculateSpeed(fix_history_[i - 1], fix_history_[i]);

        if (calculated_speed > MAX_REALISTIC_SPEED_MPS) {
            Anomaly anom = {
                .type = 0,  // Speed anomaly
                .severity = fmin(100.0f, (calculated_speed / MAX_REALISTIC_SPEED_MPS) * 50.0f),
                .timestamp = fix_history_[i].timestamp
            };
            snprintf(anom.description, sizeof(anom.description),
                    "Impossible speed: %.1f m/s (%.1f km/h) - Real aircraft ~250 m/s max",
                    calculated_speed, calculated_speed * 3.6f);
            anomalies_.push_back(anom);
            stats_.max_speed_kmh = calculated_speed * 3.6f;
        }
    }

    // ============ SIGNAL QUALITY CHECK ============
    // HDOP >100 indicates very poor signal (spoofing indicator)
    for (const auto& fix : fix_history_) {
        if (!checkSignalQuality(fix)) {
            Anomaly anom = {
                .type = 1,  // HDOP anomaly
                .severity = fmin(100.0f, (fix.hdop / MAX_REALISTIC_HDOP) * 60.0f),
                .timestamp = fix.timestamp
            };
            snprintf(anom.description, sizeof(anom.description),
                    "Signal quality degradation: HDOP=%u (normal: <50)", fix.hdop);
            anomalies_.push_back(anom);
        }
    }

    // ============ SATELLITE ANOMALY CHECK ============
    // Too few satellites (need 4+), sudden changes
    for (size_t i = 1; i < fix_history_.size(); i++) {
        if (fix_history_[i].satellites < MIN_SATELLITES) {
            Anomaly anom = {
                .type = 2,  // Satellite anomaly
                .severity = 40.0f,  // Moderate risk
                .timestamp = fix_history_[i].timestamp
            };
            snprintf(anom.description, sizeof(anom.description),
                    "Insufficient satellites: %u (need %u for 3D fix)",
                    fix_history_[i].satellites, MIN_SATELLITES);
            anomalies_.push_back(anom);
        }

        // Check for sudden satellite loss
        int sat_change = (int)fix_history_[i].satellites - (int)fix_history_[i-1].satellites;
        if (abs(sat_change) > 4) {
            Anomaly anom = {
                .type = 2,
                .severity = 50.0f,
                .timestamp = fix_history_[i].timestamp
            };
            snprintf(anom.description, sizeof(anom.description),
                    "Sudden satellite change: %+d satellites (possibly switched source)",
                    sat_change);
            anomalies_.push_back(anom);
        }
    }

    // ============ POSITION JUMP CHECK ============
    // Large jumps without corresponding speed/time
    for (size_t i = 1; i < fix_history_.size(); i++) {
        float distance = calculateDistance(fix_history_[i-1].latitude, fix_history_[i-1].longitude,
                                          fix_history_[i].latitude, fix_history_[i].longitude);

        if (distance > MAX_POSITION_JUMP) {
            Anomaly anom = {
                .type = 3,  // Jump anomaly
                .severity = 80.0f,  // High risk
                .timestamp = fix_history_[i].timestamp
            };
            snprintf(anom.description, sizeof(anom.description),
                    "Impossible position jump: %.0f meters (likely spoofed/switched)",
                    distance);
            anomalies_.push_back(anom);
        }
    }

    // ============ TRAJECTORY CONSISTENCY CHECK ============
    if (!checkConsistency(fix_history_)) {
        Anomaly anom = {
            .type = 4,  // Geometry anomaly
            .severity = 65.0f,
            .timestamp = millis() / 1000
        };
        snprintf(anom.description, sizeof(anom.description),
                "Trajectory inconsistency: Points don't form valid path (spoofing indicator)");
        anomalies_.push_back(anom);
    }

    // Count high-confidence spoofs
    stats_.anomalies_detected = anomalies_.size();
    stats_.high_confidence_spoofs = 0;
    for (const auto& anom : anomalies_) {
        if (anom.severity > 70.0f) {
            stats_.high_confidence_spoofs++;
        }
    }

    return anomalies_;
}

GPSSpoofingDetectorReal::DetectionStats GPSSpoofingDetectorReal::getStats() {
    if (!monitoring_) {
        stats_.overall_risk = calculateRiskLevel();
    }
    return stats_;
}

String GPSSpoofingDetectorReal::generateReport() {
    String report = "\n╔════════════════════════════════════════════╗\n";
    report += "║       GPS SPOOFING DETECTION REPORT        ║\n";
    report += "╚════════════════════════════════════════════╝\n\n";

    report += String("[MONITORING STATUS]\n");
    report += String("  Status:           ") + String(monitoring_ ? "ACTIVE" : "INACTIVE") + "\n";
    report += String("  Total Fixes:      ") + String(stats_.total_fixes) + "\n";
    report += String("  Duration:         ") + String(stats_.monitoring_duration) + " seconds\n";
    report += String("  Anomalies Found:  ") + String(stats_.anomalies_detected) + "\n\n";

    report += String("[SPOOFING RISK ASSESSMENT]\n");
    report += String("  Overall Risk:     ");
    if (stats_.overall_risk >= 70) {
        report += String("🚨 CRITICAL (") + String(stats_.overall_risk) + "%)\n";
    } else if (stats_.overall_risk >= 40) {
        report += String("⚠️  HIGH (") + String(stats_.overall_risk) + "%)\n";
    } else {
        report += String("✓ LOW (") + String(stats_.overall_risk) + "%)\n";
    }
    report += String("  High Confidence: ") + String(stats_.high_confidence_spoofs) + " indicators\n";
    report += String("  Max Speed:        ") + String(stats_.max_speed_kmh, 1) + " km/h\n";
    report += String("  Avg HDOP:         ") + String(stats_.avg_hdop, 1) + "\n";
    report += String("  Avg Satellites:   ") + String((int)stats_.avg_satellite_count) + "\n\n";

    if (!anomalies_.empty()) {
        report += "[DETECTED ANOMALIES]\n";
        for (size_t i = 0; i < (anomalies_.size() < 5 ? anomalies_.size() : 5); i++) {
            report += String("  • ") + anomalies_[i].description + "\n";
            report += String("    Severity: ") + String((int)anomalies_[i].severity) + "%\n";
        }
        if (anomalies_.size() > 5) {
            report += String("  ... and ") + String(anomalies_.size() - 5) + " more\n";
        }
        report += "\n";
    }

    report += "════════════════════════════════════════════\n";
    return report;
}

void GPSSpoofingDetectorReal::exportToJSON(const char* filepath) {
    if (!LittleFS.begin()) return;

    File f = LittleFS.open(filepath, FILE_WRITE);
    if (!f) {
        LittleFS.end();
        return;
    }

    f.print("{\"anomalies\":[");
    for (size_t i = 0; i < anomalies_.size(); i++) {
        if (i > 0) f.print(",");
        f.printf("{\"type\":%u,\"severity\":%.1f,\"description\":\"%s\",\"time\":%u}",
                anomalies_[i].type, anomalies_[i].severity,
                anomalies_[i].description, anomalies_[i].timestamp);
    }
    f.printf("],\"risk\":%u,\"total_fixes\":%u,\"duration\":%u}",
            stats_.overall_risk, stats_.total_fixes, stats_.monitoring_duration);

    f.close();
    LittleFS.end();

    Serial.printf("[GPSSpoofingDetector] Exported analysis to %s\n", filepath);
    AuditLog::instance().log(AuditEventType::TOOL_SUCCESS, "GPSSpoofingDetector",
                            "Analysis exported");
}

void GPSSpoofingDetectorReal::clearHistory() {
    fix_history_.clear();
    anomalies_.clear();
    fix_count_ = 0;
    memset(&stats_, 0, sizeof(DetectionStats));
    Serial.println("[GPSSpoofingDetector] History cleared");
}

float GPSSpoofingDetectorReal::calculateDistance(double lat1, double lon1, double lat2, double lon2) {
    // Haversine formula for great-circle distance
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;

    double a = sin(dLat / 2.0) * sin(dLat / 2.0) +
              cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) *
              sin(dLon / 2.0) * sin(dLon / 2.0);

    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
    return EARTH_RADIUS_M * c;
}

float GPSSpoofingDetectorReal::calculateSpeed(const GPSFix& prev, const GPSFix& curr) {
    float distance = calculateDistance(prev.latitude, prev.longitude,
                                      curr.latitude, curr.longitude);
    uint32_t time_diff = curr.timestamp - prev.timestamp;

    if (time_diff == 0) return 0.0f;
    return distance / time_diff;
}

bool GPSSpoofingDetectorReal::checkSignalQuality(const GPSFix& fix) {
    // HDOP should be between 0.5 and ~50 for normal conditions
    // Higher values indicate poor geometry/spoofing
    return (fix.hdop >= MIN_REALISTIC_HDOP && fix.hdop <= 100);
}

bool GPSSpoofingDetectorReal::checkConsistency(const std::vector<GPSFix>& history) {
    if (history.size() < 3) return true;

    // Check triangle inequality for last 3 points
    // Real trajectory should form valid triangle
    size_t sz = history.size();
    float d1 = calculateDistance(history[sz-3].latitude, history[sz-3].longitude,
                                 history[sz-2].latitude, history[sz-2].longitude);
    float d2 = calculateDistance(history[sz-2].latitude, history[sz-2].longitude,
                                 history[sz-1].latitude, history[sz-1].longitude);
    float d3 = calculateDistance(history[sz-1].latitude, history[sz-1].longitude,
                                 history[sz-3].latitude, history[sz-3].longitude);

    // Triangle inequality: sum of any two sides > third side
    return (d1 + d2 > d3 && d2 + d3 > d1 && d3 + d1 > d2);
}

bool GPSSpoofingDetectorReal::checkTrajectory(const GPSFix& prev, const GPSFix& curr) {
    // Course should be continuous (no sudden reversals)
    float course_change = fabs(curr.course - prev.course);
    if (course_change > 180.0f) {
        course_change = 360.0f - course_change;
    }

    // More than 90° change in 1 second is impossible
    return (course_change <= 90.0f);
}

uint8_t GPSSpoofingDetectorReal::calculateRiskLevel() {
    uint8_t risk = 0;

    if (anomalies_.empty()) return 0;

    // Count high-severity anomalies
    for (const auto& anom : anomalies_) {
        if (anom.severity > 80.0f) risk += 30;
        else if (anom.severity > 60.0f) risk += 20;
        else if (anom.severity > 40.0f) risk += 10;
    }

    return (risk > 100) ? 100 : risk;
}
