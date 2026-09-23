#include "gps_detector.h"
#include "audit_log.h"
#include <cmath>

#define EARTH_RADIUS_M 6371000.0

namespace GPSDetector {

struct DetectionContext {
    std::vector<GPSFix> fix_history;
    std::vector<AnomalyIndicator> anomalies;
    DetectionStats stats = {0};
};

static DetectionContext context;
static bool initialized = false;

bool begin() {
    if (initialized) return true;

    context.fix_history.clear();
    context.fix_history.reserve(200);
    context.anomalies.clear();
    memset(&context.stats, 0, sizeof(DetectionStats));

    initialized = true;
    Serial.println("[GPSDetector] GPS spoofing detection initialized");
    AuditLog::instance().log(AuditEventType::TOOL_START, "GPSDetector",
                            "GPS spoofing detection ready");
    return true;
}

float calculateDistance(double lat1, double lon1, double lat2, double lon2) {
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;

    double a = sin(dLat / 2.0) * sin(dLat / 2.0) +
              cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) *
              sin(dLon / 2.0) * sin(dLon / 2.0);

    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
    return EARTH_RADIUS_M * c;
}

float calculateSpeed(const GPSFix& prev, const GPSFix& curr) {
    float distance = calculateDistance(prev.latitude, prev.longitude,
                                      curr.latitude, curr.longitude);
    uint32_t time_diff = curr.timestamp - prev.timestamp;

    if (time_diff == 0) return 0.0f;
    return (distance / time_diff) * 3.6f;  // Convert to km/h
}

void addFix(double lat, double lon, float alt, uint8_t sats, uint16_t hdop) {
    if (!initialized) begin();

    if (context.fix_history.size() >= 200) {
        context.fix_history.erase(context.fix_history.begin());
    }

    GPSFix fix = {
        .latitude = lat,
        .longitude = lon,
        .altitude = alt,
        .satellites = sats,
        .hdop = hdop,
        .timestamp = millis() / 1000
    };

    context.fix_history.push_back(fix);
    context.stats.total_fixes++;
    context.stats.avg_hdop = (context.stats.avg_hdop + hdop) / 2.0f;
}

std::vector<AnomalyIndicator> analyzeForSpoofing() {
    context.anomalies.clear();

    if (context.fix_history.size() < 2) {
        return context.anomalies;
    }

    // Check impossible speeds
    for (size_t i = 1; i < context.fix_history.size(); i++) {
        float speed = calculateSpeed(context.fix_history[i - 1], context.fix_history[i]);

        if (speed > 360.0f) {  // >360 km/h is impossible
            AnomalyIndicator anom = {
                .type = 0,
                .severity = fmin(100.0f, (speed / 500.0f) * 100.0f),
            };
            snprintf(anom.description, sizeof(anom.description),
                    "Impossible speed: %.1f km/h (max realistic: 360 km/h)", speed);
            context.anomalies.push_back(anom);
            context.stats.max_speed_kmh = speed;
        }
    }

    // Check signal quality (HDOP)
    for (const auto& fix : context.fix_history) {
        if (fix.hdop > 100) {  // Poor signal
            AnomalyIndicator anom = {
                .type = 1,
                .severity = fmin(100.0f, (fix.hdop / 200.0f) * 100.0f),
            };
            snprintf(anom.description, sizeof(anom.description),
                    "Signal degradation: HDOP=%u (normal: <50)", fix.hdop);
            context.anomalies.push_back(anom);
        }
    }

    // Check satellite count (need 4+ for 3D fix)
    for (const auto& fix : context.fix_history) {
        if (fix.satellites < 4) {
            AnomalyIndicator anom = {
                .type = 2,
                .severity = 60.0f,
            };
            snprintf(anom.description, sizeof(anom.description),
                    "Insufficient satellites: %u (need 4+)", fix.satellites);
            context.anomalies.push_back(anom);
        }
    }

    // Check position jumps
    for (size_t i = 1; i < context.fix_history.size(); i++) {
        float distance = calculateDistance(context.fix_history[i-1].latitude,
                                          context.fix_history[i-1].longitude,
                                          context.fix_history[i].latitude,
                                          context.fix_history[i].longitude);

        if (distance > 10000.0f) {  // >10km jump
            AnomalyIndicator anom = {
                .type = 3,
                .severity = 85.0f,
            };
            snprintf(anom.description, sizeof(anom.description),
                    "Position jump: %.0f meters (suspicious)", distance);
            context.anomalies.push_back(anom);
        }
    }

    context.stats.anomalies_detected = context.anomalies.size();

    // Calculate spoofing confidence
    uint8_t confidence = 0;
    for (const auto& anom : context.anomalies) {
        if (anom.severity > 75.0f) confidence += 25;
        else if (anom.severity > 50.0f) confidence += 15;
        else confidence += 5;
    }
    context.stats.spoofing_confidence = (confidence > 100) ? 100 : confidence;

    return context.anomalies;
}

DetectionStats getStats() {
    return context.stats;
}

String generateReport() {
    String report = "\n╔════════════════════════════════════════════╗\n";
    report += "║       GPS SPOOFING DETECTION REPORT        ║\n";
    report += "╚════════════════════════════════════════════╝\n\n";

    report += String("[ANALYSIS RESULTS]\n");
    report += String("  Total Fixes:      ") + String(context.stats.total_fixes) + "\n";
    report += String("  Anomalies Found:  ") + String(context.stats.anomalies_detected) + "\n";
    report += String("  Spoofing Risk:    ") + String(context.stats.spoofing_confidence) + "%\n";
    report += String("  Max Speed:        ") + String(context.stats.max_speed_kmh, 1) + " km/h\n";
    report += String("  Avg HDOP:         ") + String(context.stats.avg_hdop, 1) + "\n\n";

    if (!context.anomalies.empty()) {
        report += "[DETECTED ANOMALIES]\n";
        for (size_t i = 0; i < context.anomalies.size() && i < 5; i++) {
            report += String("  • ") + context.anomalies[i].description + "\n";
            report += String("    Severity: ") + String((int)context.anomalies[i].severity) + "%\n";
        }
    }

    report += "\n════════════════════════════════════════════\n";
    return report;
}

void reset() {
    context.fix_history.clear();
    context.anomalies.clear();
    memset(&context.stats, 0, sizeof(DetectionStats));
    Serial.println("[GPSDetector] Detection data cleared");
}

}  // namespace GPSDetector
