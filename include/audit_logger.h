#pragma once
#include <Arduino.h>
#include <vector>
#include <ctime>

namespace AuditLogger {

// Log entry structure
struct LogEntry {
    uint32_t timestamp;
    String level;      // "ERROR", "WARN", "INFO", "SUCCESS"
    String category;   // "WiFi", "RF", "NFC", "GPS", etc.
    String message;
    String details;    // Additional data
};

// Audit result structure for export
struct AuditResult {
    String toolName;
    String category;
    uint32_t startTime;
    uint32_t endTime;
    bool success;
    String data;       // JSON formatted results
};

class Logger {
public:
    // Initialize logging system
    static void begin();

    // Log entry methods
    static void logError(const String &category, const String &message, const String &details = "");
    static void logWarning(const String &category, const String &message, const String &details = "");
    static void logInfo(const String &category, const String &message, const String &details = "");
    static void logSuccess(const String &category, const String &message, const String &details = "");

    // Audit result logging
    static void logAuditResult(const String &toolName, const String &category,
                              bool success, const String &data);

    // Export functions
    static void exportLogs(const String &filename);           // Export all logs
    static void exportAuditResults(const String &filename);   // Export audit results only
    static void exportAsCSV(const String &filename);          // CSV format
    static void exportAsJSON(const String &filename);         // JSON format

    // Query functions
    static std::vector<LogEntry> getLogs(uint32_t startTime = 0, uint32_t endTime = 0);
    static std::vector<AuditResult> getAuditResults();
    static uint32_t getLogCount();
    static void clearOldLogs(uint32_t daysOld = 7);

    // Utility
    static String formatTimestamp(uint32_t timestamp);
    static void printLogSummary();
    static void deleteAllLogs();
};

}  // namespace AuditLogger
