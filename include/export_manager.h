#ifndef EXPORT_MANAGER_H
#define EXPORT_MANAGER_H

#include <Arduino.h>
#include <LittleFS.h>
#include "audit_log.h"
#include "tool_result_persistence.h"

// Comprehensive export functionality for logs and results
class ExportManager {
public:
    enum ExportFormat {
        CSV,
        JSON,
        HTML
    };

    static ExportManager& instance() {
        static ExportManager em;
        return em;
    }

    // Export audit logs to CSV
    bool exportAuditLogsCSV(const char* output_file = "/results/export_audit.csv") {
        fs::File file = LittleFS.open(output_file, "w");
        if (!file) {
            Serial.printf("Failed to open %s for writing\n", output_file);
            return false;
        }

        // Write CSV header
        file.println("Timestamp,EventType,Module,FreeHeap,Details");

        // List audit files and read content
        fs::File audit_dir = LittleFS.open("/logs/audit");
        if (!audit_dir) {
            file.close();
            return false;
        }

        fs::File audit_file = audit_dir.openNextFile();
        uint32_t lines_written = 0;

        while (audit_file) {
            if (!audit_file.isDirectory()) {
                // Read and copy content
                while (audit_file.available()) {
                    String line = audit_file.readStringUntil('\n');
                    if (line.length() > 0) {
                        file.println(line);
                        lines_written++;
                    }
                }
            }
            audit_file = audit_dir.openNextFile();
        }

        file.close();
        Serial.printf("✓ Exported %lu audit log entries to %s\n", lines_written, output_file);
        return true;
    }

    // Export tool results to CSV
    bool exportToolResultsCSV(const char* output_file = "/results/export_tools.csv") {
        fs::File file = LittleFS.open(output_file, "w");
        if (!file) return false;

        // Write header
        file.println("Timestamp,Tool,Status,ItemsFound,DurationMs,Details");

        // Read all tool result files
        fs::File tools_dir = LittleFS.open("/results/tools");
        if (!tools_dir) {
            file.close();
            return false;
        }

        fs::File result_file = tools_dir.openNextFile();
        uint32_t files_written = 0;

        while (result_file) {
            if (!result_file.isDirectory()) {
                // Extract filename and size
                String filename = String(result_file.name());
                uint32_t size = result_file.size();

                // Simple CSV line (can be enhanced with JSON parsing)
                file.printf("%s,%u\n", filename.c_str(), size);
                files_written++;
            }
            result_file = tools_dir.openNextFile();
        }

        file.close();
        Serial.printf("✓ Exported %lu tool results to %s\n", files_written, output_file);
        return true;
    }

    // Export device discoveries to CSV
    bool exportDevicesCSV(const char* output_file = "/results/export_devices.csv") {
        fs::File file = LittleFS.open(output_file, "w");
        if (!file) return false;

        file.println("Timestamp,Tool,DeviceType,Count");

        fs::File devices_dir = LittleFS.open("/results/devices");
        if (!devices_dir) {
            file.close();
            return false;
        }

        fs::File device_file = devices_dir.openNextFile();
        uint32_t files_written = 0;

        while (device_file) {
            if (!device_file.isDirectory()) {
                file.printf("%s,%u\n", device_file.name(), device_file.size());
                files_written++;
            }
            device_file = devices_dir.openNextFile();
        }

        file.close();
        Serial.printf("✓ Exported %lu device records to %s\n", files_written, output_file);
        return true;
    }

    // Export attacks to CSV
    bool exportAttacksCSV(const char* output_file = "/results/export_attacks.csv") {
        fs::File file = LittleFS.open(output_file, "w");
        if (!file) return false;

        file.println("Timestamp,Tool,Status,SuccessCount,FailureCount,Details");

        fs::File attacks_dir = LittleFS.open("/results/attacks");
        if (!attacks_dir) {
            file.close();
            return false;
        }

        fs::File attack_file = attacks_dir.openNextFile();
        uint32_t files_written = 0;

        while (attack_file) {
            if (!attack_file.isDirectory()) {
                file.printf("%s,%u\n", attack_file.name(), attack_file.size());
                files_written++;
            }
            attack_file = attacks_dir.openNextFile();
        }

        file.close();
        Serial.printf("✓ Exported %lu attack records to %s\n", files_written, output_file);
        return true;
    }

    // Generate daily summary report
    void generateDailySummary() {
        String timestamp = String(millis() / 1000);
        String filename = "/results/report_daily_" + timestamp + ".txt";

        fs::File report = LittleFS.open(filename, "w");
        if (!report) {
            Serial.println("Failed to create daily report");
            return;
        }

        report.println("═══════════════════════════════════════");
        report.println("     DAILY SUMMARY REPORT");
        report.println("═══════════════════════════════════════\n");

        // Count files in each category
        uint32_t tools = countFilesInDir("/results/tools");
        uint32_t devices = countFilesInDir("/results/devices");
        uint32_t attacks = countFilesInDir("/results/attacks");

        report.printf("Tool Results:        %lu\n", tools);
        report.printf("Device Discoveries:  %lu\n", devices);
        report.printf("Attack Records:      %lu\n", attacks);
        report.printf("Total:               %lu\n\n", tools + devices + attacks);

        // Storage info
        uint32_t storage = getStorageUsageBytes();
        report.printf("Storage Used:        %lu bytes\n", storage);
        report.printf("Storage Usage:       %u%%\n", (storage * 100) / 500000);

        report.close();
        Serial.printf("✓ Daily summary generated: %s\n", filename.c_str());
    }

    // Generate monthly summary
    void generateMonthlySummary() {
        String timestamp = String(millis() / 1000);
        String filename = "/results/report_monthly_" + timestamp + ".txt";

        fs::File report = LittleFS.open(filename, "w");
        if (!report) {
            Serial.println("Failed to create monthly report");
            return;
        }

        report.println("═══════════════════════════════════════");
        report.println("   MONTHLY SUMMARY REPORT");
        report.println("═══════════════════════════════════════\n");

        // Detailed statistics
        uint32_t tools = countFilesInDir("/results/tools");
        uint32_t devices = countFilesInDir("/results/devices");
        uint32_t attacks = countFilesInDir("/results/attacks");

        report.printf("Period: Last 30 days\n\n");
        report.printf("Tools Executed:      %lu\n", tools);
        report.printf("Devices Found:       %lu\n", devices);
        report.printf("Attacks Performed:   %lu\n", attacks);

        // Storage trends
        report.printf("\nStorage Analysis:\n");
        report.printf("  Tools:   %.1f KB\n", getDirectorySizeKB("/results/tools"));
        report.printf("  Devices: %.1f KB\n", getDirectorySizeKB("/results/devices"));
        report.printf("  Attacks: %.1f KB\n", getDirectorySizeKB("/results/attacks"));

        report.close();
        Serial.printf("✓ Monthly summary generated: %s\n", filename.c_str());
    }

    // List all export files
    void listExportFiles() {
        Serial.println("\n╔════════════════════════════════════════╗");
        Serial.println("║          AVAILABLE EXPORTS             ║");
        Serial.println("╠════════════════════════════════════════╣");

        fs::File results_dir = LittleFS.open("/results");
        if (!results_dir) {
            Serial.println("║ No exports available                  ║");
            Serial.println("╚════════════════════════════════════════╝\n");
            return;
        }

        fs::File file = results_dir.openNextFile();
        uint32_t count = 0;

        while (file) {
            if (!file.isDirectory() && String(file.name()).indexOf("export_") == 0) {
                Serial.printf("║ %s (%u bytes)\n", file.name(), file.size());
                count++;
            }
            file = results_dir.openNextFile();
        }

        if (count == 0) {
            Serial.println("║ No exports available                  ║");
        }

        Serial.println("╚════════════════════════════════════════╝\n");
    }

private:
    // Helper: count files in directory
    uint32_t countFilesInDir(const char* path) {
        uint32_t count = 0;
        fs::File dir = LittleFS.open(path);
        if (!dir) return 0;

        fs::File file = dir.openNextFile();
        while (file) {
            if (!file.isDirectory()) count++;
            file = dir.openNextFile();
        }
        return count;
    }

    // Helper: get directory size in bytes
    uint32_t getDirectorySizeBytes(const char* path) {
        uint32_t total = 0;
        fs::File dir = LittleFS.open(path);
        if (!dir) return 0;

        fs::File file = dir.openNextFile();
        while (file) {
            if (!file.isDirectory()) total += file.size();
            file = dir.openNextFile();
        }
        return total;
    }

    // Helper: get directory size in KB
    float getDirectorySizeKB(const char* path) {
        return getDirectorySizeBytes(path) / 1024.0;
    }

    // Helper: total storage usage
    uint32_t getStorageUsageBytes() {
        return getDirectorySizeBytes("/results/tools") +
               getDirectorySizeBytes("/results/devices") +
               getDirectorySizeBytes("/results/attacks");
    }

    ExportManager() {}
};

#endif
