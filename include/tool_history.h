#ifndef TOOL_HISTORY_H
#define TOOL_HISTORY_H

#include <Arduino.h>
#include <vector>
#include <LittleFS.h>

// Track and replay tool execution history
class ToolHistory {
public:
    enum ToolType {
        SCANNER,
        ATTACKER,
        JAMMER,
        DECODER,
        DECODER_EMULATOR
    };

    struct HistoryEntry {
        String toolName;
        ToolType type;
        uint32_t timestamp;
        uint32_t executionTimeMs;
        bool success;
        String parameters;
        uint32_t itemsFound;
    };

    static ToolHistory& instance() {
        static ToolHistory th;
        return th;
    }

    // Record tool execution
    void recordExecution(const char* toolName, ToolType type,
                       uint32_t durationMs, bool success,
                       const char* params = "", uint32_t count = 0) {
        if (history.size() >= MAX_HISTORY) {
            history.erase(history.begin());  // Remove oldest
        }

        HistoryEntry entry = {
            String(toolName),
            type,
            millis(),
            durationMs,
            success,
            String(params),
            count
        };
        history.push_back(entry);

        // Persist to LittleFS
        persistEntry(entry);
    }

    // Get recent history
    std::vector<HistoryEntry> getRecent(uint32_t count = 10) const {
        std::vector<HistoryEntry> recent;
        uint32_t start = (history.size() > count) ? (history.size() - count) : 0;

        for (size_t i = start; i < history.size(); i++) {
            recent.push_back(history[i]);
        }

        return recent;
    }

    // Display history
    void displayHistory(uint32_t count = 10) {
        auto recent = getRecent(count);
        if (recent.empty()) {
            Serial.println("No history available");
            return;
        }

        Serial.println("\n╔════════════════════════════════════════╗");
        Serial.println("║          TOOL EXECUTION HISTORY       ║");
        Serial.println("╠════════════════════════════════════════╣");

        for (size_t i = 0; i < recent.size(); i++) {
            const auto& entry = recent[i];
            Serial.print("║ [");
            Serial.print(i + 1);
            Serial.print("] ");
            Serial.print(entry.toolName);
            Serial.print(" ");

            // Status icon
            if (entry.success) {
                Serial.print("✅ ");
            } else {
                Serial.print("❌ ");
            }

            // Type indicator
            switch (entry.type) {
                case SCANNER: Serial.print("[📡] "); break;
                case ATTACKER: Serial.print("[⚔️ ] "); break;
                case JAMMER: Serial.print("[🔌] "); break;
                case DECODER: Serial.print("[📊] "); break;
                case DECODER_EMULATOR: Serial.print("[🔄] "); break;
            }

            // Duration and results
            Serial.printf("%ums / %lu items\n", entry.executionTimeMs, entry.itemsFound);
        }

        Serial.println("╚════════════════════════════════════════╝\n");
    }

    // Get most frequently used tools
    std::vector<String> getFrequentTools(uint32_t limit = 5) {
        std::vector<String> tools;
        std::vector<uint32_t> counts;

        for (const auto& entry : history) {
            auto it = std::find(tools.begin(), tools.end(), entry.toolName);
            if (it != tools.end()) {
                size_t idx = std::distance(tools.begin(), it);
                counts[idx]++;
            } else {
                tools.push_back(entry.toolName);
                counts.push_back(1);
            }
        }

        // Sort by count
        for (size_t i = 0; i < tools.size(); i++) {
            for (size_t j = i + 1; j < tools.size(); j++) {
                if (counts[j] > counts[i]) {
                    std::swap(tools[i], tools[j]);
                    std::swap(counts[i], counts[j]);
                }
            }
        }

        // Return top N
        if (tools.size() > limit) {
            tools.resize(limit);
        }

        return tools;
    }

    // Display statistics
    void displayStatistics() {
        Serial.println("\n╔════════════════════════════════════════╗");
        Serial.println("║       EXECUTION STATISTICS             ║");
        Serial.println("╠════════════════════════════════════════╣");

        uint32_t total = history.size();
        uint32_t successful = 0;
        uint32_t failed = 0;
        uint64_t total_time = 0;

        for (const auto& entry : history) {
            if (entry.success) successful++;
            else failed++;
            total_time += entry.executionTimeMs;
        }

        if (total > 0) {
            uint32_t avg_time = total_time / total;
            uint8_t success_rate = (successful * 100) / total;

            Serial.printf("║ Total Executions: %lu\n", total);
            Serial.printf("║ Successful: %lu (%u%%)\n", successful, success_rate);
            Serial.printf("║ Failed: %lu (%u%%)\n", failed, 100 - success_rate);
            Serial.printf("║ Average Time: %lu ms\n", avg_time);

            // Most used tools
            auto frequent = getFrequentTools(5);
            Serial.println("║ \n║ Top Tools:");
            for (size_t i = 0; i < frequent.size(); i++) {
                Serial.printf("║   %zu. %s\n", i + 1, frequent[i].c_str());
            }
        } else {
            Serial.println("║ No execution history yet               ║");
        }

        Serial.println("╚════════════════════════════════════════╝\n");
    }

    // Clear history
    void clearHistory() {
        history.clear();
        // Also clear persisted file
        LittleFS.remove("/results/tool_history.csv");
    }

    // Get total count
    uint32_t getTotalCount() const {
        return history.size();
    }

private:
    static constexpr uint32_t MAX_HISTORY = 50;
    std::vector<HistoryEntry> history;

    ToolHistory() {
        loadHistory();
    }

    void persistEntry(const HistoryEntry& entry) {
        fs::File file = LittleFS.open("/results/tool_history.csv", "a");
        if (!file) return;

        file.printf("%lu,%s,%u,%u,%s,%lu\n",
                   entry.timestamp,
                   entry.toolName.c_str(),
                   entry.type,
                   entry.executionTimeMs,
                   entry.success ? "1" : "0",
                   entry.itemsFound);

        file.close();
    }

    void loadHistory() {
        fs::File file = LittleFS.open("/results/tool_history.csv", "r");
        if (!file) return;

        while (file.available()) {
            String line = file.readStringUntil('\n');
            if (line.length() > 0) {
                // Parse CSV: timestamp,tool,type,time,success,count
                uint32_t commaCount = 0;
                uint32_t lastIdx = 0;
                String parts[6];

                for (uint32_t i = 0; i < line.length(); i++) {
                    if (line[i] == ',' || i == line.length() - 1) {
                        parts[commaCount] = line.substring(lastIdx,
                            (i == line.length() - 1) ? i + 1 : i);
                        lastIdx = i + 1;
                        commaCount++;
                    }
                }

                if (commaCount >= 6) {
                    HistoryEntry entry = {
                        parts[1],
                        (ToolType)parts[2].toInt(),
                        parts[0].toInt(),
                        parts[3].toInt(),
                        parts[4].toInt() == 1,
                        "",
                        parts[5].toInt()
                    };
                    if (history.size() < MAX_HISTORY) {
                        history.push_back(entry);
                    }
                }
            }
        }

        file.close();
    }
};

#endif
