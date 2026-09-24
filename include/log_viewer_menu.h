#ifndef LOG_VIEWER_MENU_H
#define LOG_VIEWER_MENU_H

#include <Arduino.h>
#include "audit_log.h"
#include "tool_result_persistence.h"

// Menu interface for viewing and managing logs
class LogViewerMenu {
public:
    static LogViewerMenu& instance() {
        static LogViewerMenu lvm;
        return lvm;
    }

    // Display audit logs for current day
    void viewAuditLogs() {
        Serial.println("\n╔════════════════════════════════════════╗");
        Serial.println("║          AUDIT LOG VIEWER              ║");
        Serial.println("╚════════════════════════════════════════╝\n");

        AuditLog::instance().printReport();
    }

    // List stored tool results
    void viewToolResults() {
        Serial.println("\n╔════════════════════════════════════════╗");
        Serial.println("║        TOOL RESULTS VIEWER             ║");
        Serial.println("╚════════════════════════════════════════╝\n");

        uint32_t count = ToolResultPersistence::instance().listResults("tools");
        Serial.printf("\nTotal tool results: %u\n", count);
    }

    // List stored device discoveries
    void viewDeviceResults() {
        Serial.println("\n╔════════════════════════════════════════╗");
        Serial.println("║       DEVICE DISCOVERIES VIEWER        ║");
        Serial.println("╚════════════════════════════════════════╝\n");

        uint32_t count = ToolResultPersistence::instance().listResults("devices");
        Serial.printf("\nTotal device results: %u\n", count);
    }

    // List stored attack results
    void viewAttackResults() {
        Serial.println("\n╔════════════════════════════════════════╗");
        Serial.println("║        ATTACK RESULTS VIEWER           ║");
        Serial.println("╚════════════════════════════════════════╝\n");

        uint32_t count = ToolResultPersistence::instance().listResults("attacks");
        Serial.printf("\nTotal attack results: %u\n", count);
    }

    // Display storage statistics
    void viewStorageStats() {
        Serial.println("\n╔════════════════════════════════════════╗");
        Serial.println("║       STORAGE STATISTICS               ║");
        Serial.println("╚════════════════════════════════════════╝\n");

        uint32_t usage = ToolResultPersistence::instance().getStorageUsage();
        Serial.printf("Results storage used: %u bytes\n", usage);

        // Estimate remaining space (typical LittleFS partition)
        uint32_t total_space = 500000; // ~500KB for /results
        uint32_t remaining = (usage < total_space) ? (total_space - usage) : 0;
        uint8_t percent = (usage * 100) / total_space;

        Serial.printf("Storage usage: %u%%\n", percent);
        Serial.printf("Remaining: %u bytes\n\n", remaining);

        if (percent > 80) {
            Serial.println("⚠️  Storage nearly full! Cleanup recommended.");
        }
    }

    // Cleanup old logs
    void cleanupOldLogs(uint8_t keep_days = 7) {
        Serial.println("\n╔════════════════════════════════════════╗");
        Serial.println("║        LOG CLEANUP                     ║");
        Serial.println("╚════════════════════════════════════════╝\n");

        Serial.printf("Cleaning up logs older than %u days...\n", keep_days);

        // Cleanup audit logs
        AuditLog::instance().clearOldLogs(keep_days);

        // Cleanup result files
        uint32_t removed = ToolResultPersistence::instance().clearOldResults(keep_days);

        Serial.printf("✓ Removed %u old result files\n", removed);
        Serial.printf("✓ Old audit logs cleaned\n\n");

        viewStorageStats();
    }

    // Export audit logs to CSV
    void exportAuditLogs() {
        Serial.println("\n╔════════════════════════════════════════╗");
        Serial.println("║       EXPORTING AUDIT LOGS             ║");
        Serial.println("╚════════════════════════════════════════╝\n");

        String export_file = "/results/audit_export_" + String(millis()) + ".csv";
        Serial.printf("Exporting to: %s\n", export_file.c_str());

        // TODO: Implement audit log export to CSV
        Serial.println("Export feature coming soon...\n");
    }

    // Export tool results to CSV
    void exportToolResults() {
        Serial.println("\n╔════════════════════════════════════════╗");
        Serial.println("║    EXPORTING TOOL RESULTS              ║");
        Serial.println("╚════════════════════════════════════════╝\n");

        String export_file = "/results/tools_export_" + String(millis()) + ".csv";
        bool success = ToolResultPersistence::instance().exportResultsToCSV(
            export_file.c_str(), "tools");

        if (success) {
            Serial.printf("✓ Exported to: %s\n\n", export_file.c_str());
        } else {
            Serial.printf("✗ Export failed\n\n");
        }
    }

    // Interactive menu loop
    void showMainMenu() {
        bool running = true;

        while (running) {
            Serial.println("\n╔════════════════════════════════════════╗");
            Serial.println("║         LOG & RESULTS MANAGER          ║");
            Serial.println("╠════════════════════════════════════════╣");
            Serial.println("║ 1. View Audit Logs                     ║");
            Serial.println("║ 2. View Tool Results                   ║");
            Serial.println("║ 3. View Device Discoveries             ║");
            Serial.println("║ 4. View Attack Results                 ║");
            Serial.println("║ 5. Storage Statistics                  ║");
            Serial.println("║ 6. Cleanup Old Logs (7 days)           ║");
            Serial.println("║ 7. Export Audit Logs                   ║");
            Serial.println("║ 8. Export Tool Results                 ║");
            Serial.println("║ 0. Back to Main Menu                   ║");
            Serial.println("╚════════════════════════════════════════╝\n");

            Serial.print("Select option (0-8): ");

            // Simple menu implementation (can be enhanced with button input)
            delay(100);
            // In real implementation, this would wait for user input via buttons or serial
            // For now, just show the menu structure
            Serial.println("\n(Menu navigation to be implemented with hardware buttons)\n");
            running = false;
        }
    }

private:
    LogViewerMenu() {}
};

#endif
