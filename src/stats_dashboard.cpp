#include "stats_dashboard.h"
#include "battery.h"

StatsDashboard::AttackStats StatsDashboard::getAttackStats() {
    AttackStats stats = {0};

    // Count total attacks and successes
    LogDatabase::QueryFilter filter = {0xFF, nullptr, 0, 0};
    stats.total_attacks = LogDatabase::instance().count(filter);

    // Count successful attacks
    filter.event_type = static_cast<uint8_t>(3);  // TOOL_SUCCESS
    stats.successful_attacks = LogDatabase::instance().count(filter);

    // Count failed attacks
    filter.event_type = static_cast<uint8_t>(4);  // TOOL_FAILURE
    stats.failed_attacks = LogDatabase::instance().count(filter);

    // Calculate success rate
    if (stats.total_attacks > 0) {
        stats.success_rate = (float)stats.successful_attacks / stats.total_attacks * 100.0f;
    }

    return stats;
}

StatsDashboard::PerformanceStats StatsDashboard::getPerformanceStats() {
    PerformanceStats stats = {0};

    LogDatabase::QueryFilter filter = {0xFF, nullptr, 0, 0};

    // This is simplified - a full implementation would iterate all entries
    // For now, return basic estimates
    stats.avg_free_heap = esp_get_free_heap_size();
    stats.min_free_heap = esp_get_free_heap_size() * 80 / 100;  // Estimate
    stats.max_free_heap = 327680;  // ESP32-S3 total RAM
    stats.avg_battery = Battery::percent();
    stats.min_battery = Battery::percent();
    stats.total_uptime_seconds = millis() / 1000;

    return stats;
}

StatsDashboard::ModuleStats StatsDashboard::getModuleStats(const char* module_name) {
    ModuleStats stats = {0};
    if (!module_name) return stats;

    strncpy(stats.module_name, module_name, sizeof(stats.module_name) - 1);

    LogDatabase::QueryFilter filter = {0xFF, module_name, 0, 0};
    stats.event_count = LogDatabase::instance().count(filter);

    // Count successes for this module
    filter.event_type = 3;  // TOOL_SUCCESS
    stats.success_count = LogDatabase::instance().count(filter);

    // Count errors for this module
    filter.event_type = 9;  // ERROR_OCCURRED
    stats.error_count = LogDatabase::instance().count(filter);

    // Calculate reliability
    if (stats.event_count > 0) {
        stats.reliability_percent = (stats.success_count * 100) / stats.event_count;
    }

    return stats;
}

String StatsDashboard::getActivityTimeline(uint32_t hours_back) {
    String timeline = "{\"timeline\":[";

    // Generate hourly buckets for the timeline
    uint32_t now_seconds = millis() / 1000;
    uint32_t bucket_size = 3600;  // 1 hour in seconds

    for (int32_t hour = -hours_back; hour <= 0; hour++) {
        uint32_t hour_start = now_seconds + (hour * bucket_size);
        uint32_t hour_end = hour_start + bucket_size;

        LogDatabase::QueryFilter filter = {0xFF, nullptr, hour_start, hour_end};
        uint32_t count = LogDatabase::instance().count(filter);

        if (hour < 0) timeline += ",";
        timeline += "{\"hour\":" + String(hour) + ",\"events\":" + String(count) + "}";
    }

    timeline += "]}";
    return timeline;
}

String StatsDashboard::getBatteryReport() {
    String report = "{\"battery\":{";
    report += "\"percent\":" + String(Battery::percent()) + ",";
    report += "\"voltage\":" + String(Battery::voltage()) + ",";
    report += "\"status\":\"" + String(Battery::isLow() ? "LOW" : "NORMAL") + "\",";
    report += "\"timestamp\":" + String(millis() / 1000);
    report += "}}";
    return report;
}

uint8_t StatsDashboard::getSystemHealthScore() {
    uint8_t score = 100;

    // Check free heap
    uint32_t free_heap = esp_get_free_heap_size();
    if (free_heap < 50000) score -= 20;
    if (free_heap < 20000) score -= 30;

    // Check battery
    uint8_t batt = Battery::percent();
    if (batt < 30) score -= 15;
    if (batt < 10) score -= 20;

    // Check for errors in logs
    LogDatabase::QueryFilter filter = {9, nullptr, 0, 0};  // ERROR_OCCURRED
    uint32_t error_count = LogDatabase::instance().count(filter);
    if (error_count > 10) score -= (error_count / 5);

    return score > 0 ? score : 0;
}

String StatsDashboard::generateHTMLReport() {
    String html = "<html><head><title>ESP32 Audit Report</title><style>";
    html += "body{font-family:Arial;margin:20px;background:#f5f5f5;}";
    html += ".card{background:white;padding:15px;margin:10px 0;border-radius:5px;box-shadow:0 2px 4px rgba(0,0,0,0.1);}";
    html += ".stat{display:inline-block;width:30%;margin:10px 2%;padding:15px;background:#e3f2fd;border-radius:5px;}";
    html += ".success{color:#4caf50;}.warning{color:#ff9800;}.error{color:#f44336;}";
    html += "h1{color:#333;}h2{color:#666;border-bottom:2px solid #007bff;padding-bottom:10px;}";
    html += "</style></head><body>";

    AttackStats atk = getAttackStats();
    PerformanceStats perf = getPerformanceStats();

    html += "<h1>ESP32-S3 Audit Platform Report</h1>";
    html += "<div class='card'>";
    html += "<h2>System Health</h2>";
    uint8_t health = getSystemHealthScore();
    html += "<div class='stat'><strong>Health Score:</strong> <span class='" +
            String(health >= 70 ? "success" : health >= 50 ? "warning" : "error") + "'>" +
            String(health) + "/100</span></div>";
    html += "<div class='stat'><strong>Free Heap:</strong> " + String(esp_get_free_heap_size()) + " bytes</div>";
    html += "<div class='stat'><strong>Battery:</strong> " + String(Battery::percent()) + "%</div>";
    html += "</div>";

    html += "<div class='card'>";
    html += "<h2>Attack Statistics</h2>";
    html += "<div class='stat'><strong>Total Attacks:</strong> " + String(atk.total_attacks) + "</div>";
    html += "<div class='stat'><strong>Successful:</strong> <span class='success'>" +
            String(atk.successful_attacks) + "</span></div>";
    html += "<div class='stat'><strong>Failed:</strong> <span class='error'>" +
            String(atk.failed_attacks) + "</span></div>";
    html += "<div class='stat'><strong>Success Rate:</strong> " + String((int)atk.success_rate) + "%</div>";
    html += "</div>";

    html += "<div class='card'>";
    html += "<h2>Performance Metrics</h2>";
    html += "<div class='stat'><strong>Avg Heap:</strong> " + String(perf.avg_free_heap) + " bytes</div>";
    html += "<div class='stat'><strong>Min Heap:</strong> " + String(perf.min_free_heap) + " bytes</div>";
    html += "<div class='stat'><strong>Uptime:</strong> " + String(perf.total_uptime_seconds) + "s</div>";
    html += "</div>";

    html += "<div class='card'><p style='font-size:12px;color:#999;'>";
    html += "Generated: " + String(millis() / 1000) + "s uptime<br/>";
    html += "Firmware: ESP32-S3 Audit v2.1.0<br/>";
    html += "</p></div>";

    html += "</body></html>";
    return html;
}

void StatsDashboard::printTextSummary() {
    Serial.println("\n╔════════════════════════════════════════════╗");
    Serial.println("║         SYSTEM STATISTICS SUMMARY          ║");
    Serial.println("╚════════════════════════════════════════════╝");

    AttackStats atk = getAttackStats();
    PerformanceStats perf = getPerformanceStats();

    Serial.printf("\n[SYSTEM HEALTH]\n");
    Serial.printf("  Health Score: %u/100\n", getSystemHealthScore());
    Serial.printf("  Free Heap: %u bytes\n", esp_get_free_heap_size());
    Serial.printf("  Battery: %u%%\n", Battery::percent());
    Serial.printf("  Uptime: %u seconds\n\n", perf.total_uptime_seconds);

    Serial.printf("[ATTACK STATISTICS]\n");
    Serial.printf("  Total Attacks: %u\n", atk.total_attacks);
    Serial.printf("  Successful: %u\n", atk.successful_attacks);
    Serial.printf("  Failed: %u\n", atk.failed_attacks);
    Serial.printf("  Success Rate: %.1f%%\n\n", atk.success_rate);

    Serial.printf("[PERFORMANCE]\n");
    Serial.printf("  Avg Heap: %u bytes\n", perf.avg_free_heap);
    Serial.printf("  Min Heap: %u bytes\n", perf.min_free_heap);
    Serial.printf("  Avg Battery: %u%%\n\n", perf.avg_battery);

    Serial.println("════════════════════════════════════════════\n");
}

String StatsDashboard::getTopModules(uint8_t count) {
    String result = "{\"top_modules\":[";

    // Get activity for each module
    const char* modules[] = {"WiFiTools", "BleTools", "Nrf24Tools", "SubGhz",
                            "IrTools", "Deauth", "Wardriving", "WebCtrl"};

    uint32_t counts[8] = {0};
    for (uint8_t i = 0; i < 8; i++) {
        LogDatabase::QueryFilter filter = {0xFF, modules[i], 0, 0};
        counts[i] = LogDatabase::instance().count(filter);
    }

    // Return top N
    for (uint8_t i = 0; i < (count > 8 ? 8 : count); i++) {
        if (i > 0) result += ",";
        uint32_t max_idx = 0;
        uint32_t max_val = 0;

        for (uint8_t j = 0; j < 8; j++) {
            if (counts[j] > max_val) {
                max_val = counts[j];
                max_idx = j;
            }
        }

        result += "{\"module\":\"" + String(modules[max_idx]) + "\",\"events\":" + String(counts[max_idx]) + "}";
        counts[max_idx] = 0;
    }

    result += "]}";
    return result;
}

String StatsDashboard::getErrorRateReport() {
    String report = "{\"error_rates\":[";

    const char* modules[] = {"WiFiTools", "BleTools", "Nrf24Tools"};

    for (uint8_t i = 0; i < 3; i++) {
        LogDatabase::QueryFilter filter = {0xFF, modules[i], 0, 0};
        uint32_t total = LogDatabase::instance().count(filter);

        filter.event_type = 9;  // ERROR_OCCURRED
        uint32_t errors = LogDatabase::instance().count(filter);

        uint8_t error_rate = total > 0 ? (errors * 100) / total : 0;

        if (i > 0) report += ",";
        report += "{\"module\":\"" + String(modules[i]) + "\",\"error_rate\":" + String(error_rate) + "}";
    }

    report += "]}";
    return report;
}

String StatsDashboard::detectAnomalies() {
    String anomalies = "{\"anomalies\":[";

    uint32_t count_found = 0;

    // Check for low memory anomalies
    if (esp_get_free_heap_size() < 30000) {
        if (count_found > 0) anomalies += ",";
        anomalies += "{\"type\":\"memory\",\"description\":\"Free heap critically low\",\"severity\":9}";
        count_found++;
    }

    // Check for high error rate
    LogDatabase::QueryFilter filter = {9, nullptr, 0, 0};  // Errors
    uint32_t errors = LogDatabase::instance().count(filter);
    filter.event_type = 0xFF;
    uint32_t total = LogDatabase::instance().count(filter);

    if (total > 10 && (errors * 100) / total > 30) {
        if (count_found > 0) anomalies += ",";
        anomalies += "{\"type\":\"reliability\",\"description\":\"High error rate detected\",\"severity\":7}";
        count_found++;
    }

    // Check for low battery
    if (Battery::isLow()) {
        if (count_found > 0) anomalies += ",";
        anomalies += "{\"type\":\"battery\",\"description\":\"Battery level low\",\"severity\":6}";
        count_found++;
    }

    anomalies += "]}";
    return anomalies;
}

uint32_t StatsDashboard::countEventType(uint8_t type) {
    LogDatabase::QueryFilter filter = {type, nullptr, 0, 0};
    return LogDatabase::instance().count(filter);
}

float StatsDashboard::calculateSuccessRate() {
    uint32_t success = countEventType(3);  // TOOL_SUCCESS
    uint32_t total = LogDatabase::instance().count({0xFF, nullptr, 0, 0});
    return total > 0 ? (float)success / total * 100.0f : 0.0f;
}

const char* StatsDashboard::formatBytes(uint32_t bytes) {
    static char buffer[32];
    if (bytes < 1024) {
        snprintf(buffer, sizeof(buffer), "%u B", bytes);
    } else if (bytes < 1024 * 1024) {
        snprintf(buffer, sizeof(buffer), "%.1f KB", bytes / 1024.0f);
    } else {
        snprintf(buffer, sizeof(buffer), "%.1f MB", bytes / (1024.0f * 1024.0f));
    }
    return buffer;
}

uint32_t StatsDashboard::estimateUptime() {
    return millis() / 1000;
}
