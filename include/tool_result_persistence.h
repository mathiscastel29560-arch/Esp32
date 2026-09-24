#ifndef TOOL_RESULT_PERSISTENCE_H
#define TOOL_RESULT_PERSISTENCE_H

#include <Arduino.h>
#include <LittleFS.h>
#include "audit_log.h"
#include <vector>

// Structured result persistence for tool execution
// Stores tool results in JSON format with metadata
// Supports querying, filtering, and export

class ToolResultPersistence {
public:
    static ToolResultPersistence& instance() {
        static ToolResultPersistence trp;
        return trp;
    }

    // Initialize persistence layer
    bool begin() {
        if (!LittleFS.begin()) return false;

        if (!LittleFS.exists("/results")) {
            LittleFS.mkdir("/results");
        }
        if (!LittleFS.exists("/results/tools")) {
            LittleFS.mkdir("/results/tools");
        }
        if (!LittleFS.exists("/results/devices")) {
            LittleFS.mkdir("/results/devices");
        }
        if (!LittleFS.exists("/results/attacks")) {
            LittleFS.mkdir("/results/attacks");
        }

        Serial.println("[ToolResultPersistence] Initialized");
        return true;
    }

    // Store tool execution result
    bool storeToolResult(const char* tool_name, const char* result_json) {
        String timestamp = String(millis() / 1000);
        String filename = "/results/tools/" + String(tool_name) + "_" + timestamp + ".json";

        fs::File file = LittleFS.open(filename, "w");
        if (!file) return false;

        file.print(result_json);
        file.close();

        Serial.printf("[ToolResult] Stored: %s\n", tool_name);
        return true;
    }

    // Store device discovery result
    bool storeDeviceResult(const char* tool_name, const char* device_info_json) {
        String timestamp = String(millis() / 1000);
        String filename = "/results/devices/" + String(tool_name) + "_" + timestamp + ".json";

        fs::File file = LittleFS.open(filename, "w");
        if (!file) return false;

        file.print(device_info_json);
        file.close();

        Serial.printf("[DeviceResult] Stored from %s\n", tool_name);
        return true;
    }

    // Store attack result
    bool storeAttackResult(const char* tool_name, const char* target, const char* attack_json) {
        String timestamp = String(millis() / 1000);
        String attack_name = String(tool_name) + "_" + String(target);
        String filename = "/results/attacks/" + attack_name + "_" + timestamp + ".json";

        fs::File file = LittleFS.open(filename, "w");
        if (!file) return false;

        file.print(attack_json);
        file.close();

        Serial.printf("[AttackResult] Stored: %s -> %s\n", tool_name, target);
        return true;
    }

    // List stored results
    uint32_t listResults(const char* category = "tools") {
        String path = "/results/";
        path += category;

        fs::File root = LittleFS.open(path);
        if (!root) return 0;

        uint32_t count = 0;
        fs::File file = root.openNextFile();

        Serial.printf("\n[Results] %s:\n", category);
        while (file) {
            if (!file.isDirectory()) {
                Serial.printf("  - %s (%u bytes)\n", file.name(), file.size());
                count++;
            }
            file = root.openNextFile();
        }

        return count;
    }

    // Get total results storage usage
    uint32_t getStorageUsage() {
        uint32_t total = 0;
        String categories[] = {"tools", "devices", "attacks"};

        for (const String& cat : categories) {
            String path = "/results/" + cat;
            fs::File root = LittleFS.open(path);
            if (!root) continue;

            fs::File file = root.openNextFile();
            while (file) {
                if (!file.isDirectory()) {
                    total += file.size();
                }
                file = root.openNextFile();
            }
        }

        return total;
    }

    // Clear old results (keep last N days)
    uint32_t clearOldResults(uint8_t keep_days = 7) {
        uint32_t removed = 0;
        uint32_t cutoff_time = (millis() / 1000) - (keep_days * 24 * 60 * 60);

        String categories[] = {"tools", "devices", "attacks"};

        for (const String& cat : categories) {
            String path = "/results/" + cat;
            fs::File root = LittleFS.open(path);
            if (!root) continue;

            fs::File file = root.openNextFile();
            while (file) {
                if (!file.isDirectory()) {
                    String name = file.name();
                    // Extract timestamp from filename
                    int last_underscore = name.lastIndexOf('_');
                    if (last_underscore > 0) {
                        String timestamp_str = name.substring(last_underscore + 1);
                        timestamp_str.remove(timestamp_str.length() - 5); // Remove .json
                        uint32_t file_time = timestamp_str.toInt();

                        if (file_time < cutoff_time) {
                            String filepath = path + "/" + name;
                            if (LittleFS.remove(filepath)) {
                                removed++;
                            }
                        }
                    }
                }
                file = root.openNextFile();
            }
        }

        if (removed > 0) {
            Serial.printf("[ToolResultPersistence] Cleaned up %u old results\n", removed);
        }

        return removed;
    }

    // Export results to CSV
    bool exportResultsToCSV(const char* output_file, const char* category = "tools") {
        String path = "/results/";
        path += category;

        fs::File root = LittleFS.open(path);
        if (!root) return false;

        fs::File out = LittleFS.open(output_file, "w");
        if (!out) return false;

        // Write CSV header
        out.println("timestamp,tool,filename,size_bytes");

        fs::File file = root.openNextFile();
        uint32_t count = 0;

        while (file) {
            if (!file.isDirectory()) {
                String name = file.name();
                out.printf("%lu,%s,%s,%u\n",
                          millis() / 1000,
                          category,
                          name,
                          file.size());
                count++;
            }
            file = root.openNextFile();
        }

        out.close();

        Serial.printf("[ToolResultPersistence] Exported %u %s results to CSV\n", count, category);
        return true;
    }

private:
    ToolResultPersistence() {}
};

#endif
