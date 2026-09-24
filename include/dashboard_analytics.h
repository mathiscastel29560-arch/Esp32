#ifndef DASHBOARD_ANALYTICS_H
#define DASHBOARD_ANALYTICS_H

#include <Arduino.h>
#include <vector>
#include <cmath>

// Advanced analytics and visualization for tool execution metrics
class DashboardAnalytics {
public:
    static DashboardAnalytics& instance() {
        static DashboardAnalytics da;
        return da;
    }

    // Signal strength visualization (RSSI indicator)
    String visualizeSignalStrength(int8_t rssi) {
        // Convert RSSI to visual representation
        if (rssi >= -30) {
            return "████████░░ Excellent (-30 dBm)";
        } else if (rssi >= -60) {
            return "██████░░░░ Good (-60 dBm)";
        } else if (rssi >= -80) {
            return "████░░░░░░ Fair (-80 dBm)";
        } else if (rssi >= -100) {
            return "██░░░░░░░░ Poor (-100 dBm)";
        } else {
            return "░░░░░░░░░░ Very Poor (< -100 dBm)";
        }
    }

    // Channel distribution bar chart
    void displayChannelAnalysis(const uint32_t* channelCounts, uint32_t channelCount) {
        Serial.println("\n╔════════════════════════════════════════╗");
        Serial.println("║       CHANNEL DISTRIBUTION             ║");
        Serial.println("╠════════════════════════════════════════╣");

        // Find max for scaling
        uint32_t max_count = 0;
        for (uint32_t i = 0; i < channelCount; i++) {
            if (channelCounts[i] > max_count) max_count = channelCounts[i];
        }

        if (max_count == 0) {
            Serial.println("║ No data available                      ║");
        } else {
            for (uint32_t i = 0; i < channelCount && i < 14; i++) {
                Serial.printf("║ Ch %2lu: ", i + 1);

                // Draw bar
                uint32_t bar_width = (channelCounts[i] * 20) / max_count;
                for (uint32_t j = 0; j < bar_width; j++) Serial.print("█");
                for (uint32_t j = bar_width; j < 20; j++) Serial.print("░");

                Serial.printf(" %lu\n", channelCounts[i]);
            }
        }

        Serial.println("╚════════════════════════════════════════╝\n");
    }

    // Device type distribution
    void displayDeviceDistribution(const char* categories[], const uint32_t* counts, uint32_t count) {
        Serial.println("\n╔════════════════════════════════════════╗");
        Serial.println("║       DEVICE TYPE DISTRIBUTION         ║");
        Serial.println("╠════════════════════════════════════════╣");

        uint32_t total = 0;
        for (uint32_t i = 0; i < count; i++) {
            total += counts[i];
        }

        if (total == 0) {
            Serial.println("║ No devices found                       ║");
        } else {
            for (uint32_t i = 0; i < count && i < 10; i++) {
                uint8_t percent = (counts[i] * 100) / total;

                Serial.print("║ ");
                Serial.print(categories[i]);
                Serial.print(": ");

                // Percent bar
                uint32_t bar_width = percent / 5;
                for (uint32_t j = 0; j < bar_width; j++) Serial.print("█");
                for (uint32_t j = bar_width; j < 20; j++) Serial.print("░");

                Serial.printf(" %2u%% (%lu)\n", percent, counts[i]);
            }
        }

        Serial.println("╚════════════════════════════════════════╝\n");
    }

    // Attack success rate graph
    void displaySuccessRateChart(uint32_t successCount, uint32_t failureCount) {
        Serial.println("\n╔════════════════════════════════════════╗");
        Serial.println("║       ATTACK SUCCESS METRICS           ║");
        Serial.println("╠════════════════════════════════════════╣");

        uint32_t total = successCount + failureCount;
        if (total == 0) {
            Serial.println("║ No attacks recorded                    ║");
        } else {
            uint8_t success_percent = (successCount * 100) / total;
            uint8_t failure_percent = (failureCount * 100) / total;

            Serial.print("║ Success: ");
            uint32_t success_bar = success_percent / 5;
            for (uint32_t i = 0; i < success_bar; i++) Serial.print("█");
            for (uint32_t i = success_bar; i < 20; i++) Serial.print("░");
            Serial.printf(" %3u%% (%lu)\n", success_percent, successCount);

            Serial.print("║ Failure: ");
            uint32_t failure_bar = failure_percent / 5;
            for (uint32_t i = 0; i < failure_bar; i++) Serial.print("█");
            for (uint32_t i = failure_bar; i < 20; i++) Serial.print("░");
            Serial.printf(" %3u%% (%lu)\n", failure_percent, failureCount);
        }

        Serial.println("╚════════════════════════════════════════╝\n");
    }

    // Time-based performance graph
    void displayPerformanceGraph(const uint32_t* durations, uint32_t count) {
        if (count == 0) {
            Serial.println("No performance data available");
            return;
        }

        Serial.println("\n╔════════════════════════════════════════╗");
        Serial.println("║       EXECUTION TIME ANALYSIS          ║");
        Serial.println("╠════════════════════════════════════════╣");

        // Calculate statistics
        uint32_t min_time = durations[0];
        uint32_t max_time = durations[0];
        uint64_t total_time = 0;

        for (uint32_t i = 0; i < count; i++) {
            if (durations[i] < min_time) min_time = durations[i];
            if (durations[i] > max_time) max_time = durations[i];
            total_time += durations[i];
        }

        uint32_t avg_time = total_time / count;

        Serial.printf("║ Executions: %lu\n", count);
        Serial.printf("║ Min Time:   %lu ms\n", min_time);
        Serial.printf("║ Max Time:   %lu ms\n", max_time);
        Serial.printf("║ Avg Time:   %lu ms\n", avg_time);

        // Show execution timeline
        Serial.println("║ \n║ Timeline (last 10):");
        uint32_t start_idx = (count > 10) ? (count - 10) : 0;
        for (uint32_t i = start_idx; i < count; i++) {
            uint32_t bar_width = (durations[i] * 15) / max_time;
            Serial.print("║ ");
            for (uint32_t j = 0; j < bar_width; j++) Serial.print("█");
            for (uint32_t j = bar_width; j < 15; j++) Serial.print("░");
            Serial.printf(" %lu ms\n", durations[i]);
        }

        Serial.println("╚════════════════════════════════════════╝\n");
    }

    // Heatmap for frequency analysis
    void displayFrequencyHeatmap(const uint32_t* frequencies, uint32_t count) {
        if (count == 0) return;

        Serial.println("\n╔════════════════════════════════════════╗");
        Serial.println("║       FREQUENCY ACTIVITY HEATMAP       ║");
        Serial.println("╠════════════════════════════════════════╣");

        // Find max for scaling
        uint32_t max_activity = 0;
        for (uint32_t i = 0; i < count; i++) {
            if (frequencies[i] > max_activity) max_activity = frequencies[i];
        }

        if (max_activity == 0) {
            Serial.println("║ No frequency data                      ║");
        } else {
            // Display as heatmap intensity
            for (uint32_t i = 0; i < count && i < 14; i++) {
                Serial.printf("║ %2lu MHz: ", i);

                uint8_t intensity = (frequencies[i] * 100) / max_activity;
                String heat = getHeatmapSymbol(intensity);

                for (int j = 0; j < 20; j++) {
                    Serial.print(heat);
                }

                Serial.printf(" %lu%%\n", intensity);
            }
        }

        Serial.println("╚════════════════════════════════════════╝\n");
    }

    // Network topology visualization
    void displayNetworkTopology(uint32_t deviceCount, uint32_t avgSignal) {
        Serial.println("\n╔════════════════════════════════════════╗");
        Serial.println("║        NETWORK TOPOLOGY SKETCH         ║");
        Serial.println("╠════════════════════════════════════════╣");

        Serial.println("║                 DEVICE                 ║");
        Serial.println("║                   |                   ║");
        Serial.printf("║            (ESP32 @ %d dBm)\n", avgSignal);
        Serial.println("║          /        |        \\          ║");

        // Show connected devices
        for (uint32_t i = 0; i < deviceCount && i < 5; i++) {
            Serial.printf("║        [DEV %lu]  [DEV %lu]  [DEV %lu]    ║\n",
                         i+1, i+2, i+3);
        }

        if (deviceCount > 5) {
            Serial.printf("║        ... and %lu more devices\n", deviceCount - 5);
        }

        Serial.println("╚════════════════════════════════════════╝\n");
    }

private:
    String getHeatmapSymbol(uint8_t intensity) {
        if (intensity >= 80) return "🟥";      // Hot
        if (intensity >= 60) return "🟧";      // Warm
        if (intensity >= 40) return "🟨";      // Medium
        if (intensity >= 20) return "🟦";      // Cool
        return "⬜";                           // Cold
    }

    DashboardAnalytics() {}
};

#endif
