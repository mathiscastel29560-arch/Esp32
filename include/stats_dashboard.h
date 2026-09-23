#ifndef STATS_DASHBOARD_H
#define STATS_DASHBOARD_H

#include <Arduino.h>
#include "log_database.h"

// Statistics and analytics dashboard
// Analyzes logs to provide insights on attacks, performance, and device health

class StatsDashboard {
public:
    static StatsDashboard& instance() {
        static StatsDashboard sd;
        return sd;
    }

    // Statistics structures
    struct AttackStats {
        uint32_t total_attacks;
        uint32_t successful_attacks;
        uint32_t failed_attacks;
        float success_rate;
        uint32_t most_used_attack;  // Most frequent event type
        uint32_t most_targeted_module;  // Module with most activity
    };

    struct PerformanceStats {
        uint32_t avg_free_heap;
        uint32_t min_free_heap;
        uint32_t max_free_heap;
        uint8_t avg_battery;
        uint8_t min_battery;
        uint32_t total_uptime_seconds;
    };

    struct ModuleStats {
        char module_name[32];
        uint32_t event_count;
        uint32_t success_count;
        uint32_t error_count;
        uint8_t reliability_percent;
    };

    struct TimeSeriesData {
        uint32_t timestamp;
        uint16_t value;
    };

    // Generate overall statistics
    AttackStats getAttackStats();

    // Performance metrics
    PerformanceStats getPerformanceStats();

    // Get stats for specific module
    ModuleStats getModuleStats(const char* module_name);

    // Get hourly activity timeline
    String getActivityTimeline(uint32_t hours_back = 24);

    // Get battery health report
    String getBatteryReport();

    // Get system health score (0-100)
    uint8_t getSystemHealthScore();

    // Generate full HTML report
    String generateHTMLReport();

    // Generate text summary for serial output
    void printTextSummary();

    // Get top N modules by activity
    String getTopModules(uint8_t count = 5);

    // Get error rate by module
    String getErrorRateReport();

    // Detect anomalies (unusual patterns)
    struct Anomaly {
        const char* type;
        const char* description;
        uint8_t severity;  // 0-10
    };

    String detectAnomalies();

private:
    StatsDashboard() {}

    // Helper: count events of type
    uint32_t countEventType(uint8_t type);

    // Helper: calculate success rate
    float calculateSuccessRate();

    // Helper: format bytes to readable format
    const char* formatBytes(uint32_t bytes);

    // Helper: estimate uptime
    uint32_t estimateUptime();
};

#endif
