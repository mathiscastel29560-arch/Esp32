#ifndef RECENT_RESULTS_TRACKER_H
#define RECENT_RESULTS_TRACKER_H

#include <Arduino.h>
#include <vector>

// Track and display recent tool executions
class RecentResultsTracker {
public:
    enum ResultStatus {
        PENDING,
        SUCCESS,
        FAILURE,
        PARTIAL
    };

    struct Result {
        String toolName;
        uint32_t timestamp;
        ResultStatus status;
        uint32_t itemsFound;    // devices, packets, targets, etc.
        uint32_t durationMs;
        String details;
    };

    static RecentResultsTracker& instance() {
        static RecentResultsTracker rrt;
        return rrt;
    }

    // Record a tool result
    void recordResult(const char* toolName, ResultStatus status,
                     uint32_t itemsFound, uint32_t durationMs,
                     const char* details = "") {
        if (results.size() >= MAX_RESULTS) {
            results.erase(results.begin());  // Remove oldest
        }

        Result result = {
            String(toolName),
            millis(),
            status,
            itemsFound,
            durationMs,
            String(details)
        };
        results.push_back(result);
    }

    // Quick helpers
    void recordSuccess(const char* tool, uint32_t count, uint32_t ms) {
        recordResult(tool, SUCCESS, count, ms);
    }

    void recordFailure(const char* tool, const char* reason) {
        recordResult(tool, FAILURE, 0, 0, reason);
    }

    void recordPartial(const char* tool, uint32_t count, uint32_t ms, const char* reason) {
        recordResult(tool, PARTIAL, count, ms, reason);
    }

    // Get most recent N results
    std::vector<Result> getRecent(uint32_t count = 5) const {
        std::vector<Result> recent;
        uint32_t start = (results.size() > count) ? (results.size() - count) : 0;

        for (size_t i = start; i < results.size(); i++) {
            recent.push_back(results[i]);
        }

        return recent;
    }

    // Display recent results
    void displayRecent(uint32_t count = 5) {
        auto recent = getRecent(count);
        if (recent.empty()) {
            Serial.println("No recent results\n");
            return;
        }

        Serial.println("\n╔════════════════════════════════════════╗");
        Serial.println("║          RECENT RESULTS (5)            ║");
        Serial.println("╠════════════════════════════════════════╣");

        for (size_t i = 0; i < recent.size(); i++) {
            const auto& result = recent[i];
            Serial.print("║ [");
            Serial.print(i + 1);
            Serial.print("] ");

            switch (result.status) {
                case SUCCESS:  Serial.print("✅ "); break;
                case FAILURE:  Serial.print("❌ "); break;
                case PARTIAL:  Serial.print("⚠️  "); break;
                case PENDING:  Serial.print("⏳ "); break;
            }

            Serial.print(result.toolName);
            Serial.println(String(35 - result.toolName.length(), ' ') + "║");

            // Details line
            Serial.print("║   ");
            if (result.itemsFound > 0) {
                Serial.printf("Found: %u | ", result.itemsFound);
            }
            Serial.printf("Time: %ums", result.durationMs);

            if (result.details.length() > 0) {
                Serial.printf(" | %s", result.details.c_str());
            }

            Serial.println();
        }

        Serial.println("╚════════════════════════════════════════╝\n");
    }

    // Get stats
    uint32_t getSuccessCount() const {
        uint32_t count = 0;
        for (const auto& r : results) {
            if (r.status == SUCCESS) count++;
        }
        return count;
    }

    uint32_t getFailureCount() const {
        uint32_t count = 0;
        for (const auto& r : results) {
            if (r.status == FAILURE) count++;
        }
        return count;
    }

    uint32_t getTotalResults() const {
        return results.size();
    }

    // Get success rate
    uint8_t getSuccessRate() const {
        if (results.empty()) return 0;
        return (getSuccessCount() * 100) / results.size();
    }

    // Clear results
    void clearAll() {
        results.clear();
    }

private:
    static constexpr uint32_t MAX_RESULTS = 32;
    std::vector<Result> results;

    RecentResultsTracker() {}
};

#endif
