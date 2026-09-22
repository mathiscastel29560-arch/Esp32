#include "audit_logger.h"
#include "debug_logger.h"
#include "rtc_clock.h"
#include <LittleFS.h>
#include <ctime>

namespace AuditLogger {

// Constants
const char *LOG_DIR = "/logs";
const char *AUDIT_LOG_FILE = "/logs/audit_log.txt";
const char *AUDIT_RESULTS_FILE = "/logs/audit_results.json";
const size_t MAX_LOG_SIZE = 1000000;  // 1MB max per log file

static std::vector<LogEntry> g_logBuffer;
static std::vector<AuditResult> g_auditResults;

void Logger::begin() {
    // Initialize LittleFS
    if (!LittleFS.begin()) {
        Serial.println("ERROR: LittleFS mount failed");
        return;
    }

    // Create logs directory if needed
    File dir = LittleFS.open(LOG_DIR, "r");
    if (!dir) {
        LittleFS.mkdir(LOG_DIR);
    }

    DBG_INFO("AuditLogger", "Logging system initialized");
}

void Logger::logError(const String &category, const String &message, const String &details) {
    LogEntry entry;
    entry.timestamp = time(nullptr);
    entry.level = "ERROR";
    entry.category = category;
    entry.message = message;
    entry.details = details;

    g_logBuffer.push_back(entry);

    // Also print to serial for debugging
    Serial.print(COLOR_RED);
    Serial.print("[ERROR] ");
    Serial.print(category);
    Serial.print(": ");
    Serial.println(message);
    Serial.print(COLOR_RESET);

    // Append to file
    File file = LittleFS.open(AUDIT_LOG_FILE, "a");
    if (file) {
        file.print("[ERROR] ");
        file.print(formatTimestamp(entry.timestamp));
        file.print(" | ");
        file.print(category);
        file.print(" | ");
        file.print(message);
        if (details.length() > 0) {
            file.print(" | ");
            file.print(details);
        }
        file.println();
        file.close();
    }
}

void Logger::logWarning(const String &category, const String &message, const String &details) {
    LogEntry entry;
    entry.timestamp = time(nullptr);
    entry.level = "WARN";
    entry.category = category;
    entry.message = message;
    entry.details = details;

    g_logBuffer.push_back(entry);

    Serial.print(COLOR_YELLOW);
    Serial.print("[WARN] ");
    Serial.print(category);
    Serial.print(": ");
    Serial.println(message);
    Serial.print(COLOR_RESET);

    File file = LittleFS.open(AUDIT_LOG_FILE, "a");
    if (file) {
        file.print("[WARN] ");
        file.print(formatTimestamp(entry.timestamp));
        file.print(" | ");
        file.print(category);
        file.print(" | ");
        file.print(message);
        if (details.length() > 0) {
            file.print(" | ");
            file.print(details);
        }
        file.println();
        file.close();
    }
}

void Logger::logInfo(const String &category, const String &message, const String &details) {
    LogEntry entry;
    entry.timestamp = time(nullptr);
    entry.level = "INFO";
    entry.category = category;
    entry.message = message;
    entry.details = details;

    g_logBuffer.push_back(entry);

    Serial.print(COLOR_CYAN);
    Serial.print("[INFO] ");
    Serial.print(category);
    Serial.print(": ");
    Serial.println(message);
    Serial.print(COLOR_RESET);

    File file = LittleFS.open(AUDIT_LOG_FILE, "a");
    if (file) {
        file.print("[INFO] ");
        file.print(formatTimestamp(entry.timestamp));
        file.print(" | ");
        file.print(category);
        file.print(" | ");
        file.print(message);
        if (details.length() > 0) {
            file.print(" | ");
            file.print(details);
        }
        file.println();
        file.close();
    }
}

void Logger::logSuccess(const String &category, const String &message, const String &details) {
    LogEntry entry;
    entry.timestamp = time(nullptr);
    entry.level = "SUCCESS";
    entry.category = category;
    entry.message = message;
    entry.details = details;

    g_logBuffer.push_back(entry);

    Serial.print(COLOR_GREEN);
    Serial.print("[✓] ");
    Serial.print(category);
    Serial.print(": ");
    Serial.println(message);
    Serial.print(COLOR_RESET);

    File file = LittleFS.open(AUDIT_LOG_FILE, "a");
    if (file) {
        file.print("[SUCCESS] ");
        file.print(formatTimestamp(entry.timestamp));
        file.print(" | ");
        file.print(category);
        file.print(" | ");
        file.print(message);
        if (details.length() > 0) {
            file.print(" | ");
            file.print(details);
        }
        file.println();
        file.close();
    }
}

void Logger::logAuditResult(const String &toolName, const String &category,
                           bool success, const String &data) {
    AuditResult result;
    result.toolName = toolName;
    result.category = category;
    result.startTime = time(nullptr) - 60;  // Approximate start (1 min ago)
    result.endTime = time(nullptr);
    result.success = success;
    result.data = data;

    g_auditResults.push_back(result);

    // Append to results file
    File file = LittleFS.open(AUDIT_RESULTS_FILE, "a");
    if (file) {
        file.print("{\"tool\":\"");
        file.print(toolName);
        file.print("\",\"category\":\"");
        file.print(category);
        file.print("\",\"timestamp\":");
        file.print(result.endTime);
        file.print(",\"success\":");
        file.print(success ? "true" : "false");
        file.print(",\"data\":");
        file.print(data);
        file.println("}");
        file.close();
    }
}

void Logger::exportLogs(const String &filename) {
    File srcFile = LittleFS.open(AUDIT_LOG_FILE, "r");
    if (!srcFile) {
        DBG_ERROR("AuditLogger", "Source log file not found");
        return;
    }

    File dstFile = LittleFS.open(filename, "w");
    if (!dstFile) {
        DBG_ERROR("AuditLogger", "Could not create export file");
        srcFile.close();
        return;
    }

    // Copy content
    uint8_t buffer[256];
    size_t bytesRead;
    while ((bytesRead = srcFile.read(buffer, sizeof(buffer))) > 0) {
        dstFile.write(buffer, bytesRead);
    }

    srcFile.close();
    dstFile.close();

    DBG_INFO("AuditLogger", "Logs exported to " + filename);
}

void Logger::exportAsCSV(const String &filename) {
    File file = LittleFS.open(filename, "w");
    if (!file) {
        DBG_ERROR("AuditLogger", "Could not create CSV file");
        return;
    }

    // Write header
    file.println("Timestamp,Level,Category,Message,Details");

    // Write log entries
    for (const auto &entry : g_logBuffer) {
        file.print(formatTimestamp(entry.timestamp));
        file.print(",");
        file.print(entry.level);
        file.print(",");
        file.print(entry.category);
        file.print(",\"");
        file.print(entry.message);
        file.print("\",\"");
        file.print(entry.details);
        file.println("\"");
    }

    file.close();
    DBG_INFO("AuditLogger", "CSV exported to " + filename);
}

void Logger::exportAsJSON(const String &filename) {
    File file = LittleFS.open(filename, "w");
    if (!file) {
        DBG_ERROR("AuditLogger", "Could not create JSON file");
        return;
    }

    file.println("{");
    file.println("  \"audit_logs\": [");

    for (size_t i = 0; i < g_logBuffer.size(); i++) {
        const auto &entry = g_logBuffer[i];
        file.print("    {");
        file.print("\"timestamp\":");
        file.print(entry.timestamp);
        file.print(",\"level\":\"");
        file.print(entry.level);
        file.print("\",\"category\":\"");
        file.print(entry.category);
        file.print("\",\"message\":\"");
        file.print(entry.message);
        file.print("\"");

        if (entry.details.length() > 0) {
            file.print(",\"details\":\"");
            file.print(entry.details);
            file.print("\"");
        }

        file.print("}");
        if (i < g_logBuffer.size() - 1) file.print(",");
        file.println();
    }

    file.println("  ]");
    file.println("}");
    file.close();

    DBG_INFO("AuditLogger", "JSON exported to " + filename);
}

std::vector<LogEntry> Logger::getLogs(uint32_t startTime, uint32_t endTime) {
    std::vector<LogEntry> result;

    for (const auto &entry : g_logBuffer) {
        if (startTime == 0 || entry.timestamp >= startTime) {
            if (endTime == 0 || entry.timestamp <= endTime) {
                result.push_back(entry);
            }
        }
    }

    return result;
}

std::vector<AuditResult> Logger::getAuditResults() {
    return g_auditResults;
}

uint32_t Logger::getLogCount() {
    return g_logBuffer.size();
}

void Logger::clearOldLogs(uint32_t daysOld) {
    uint32_t cutoffTime = time(nullptr) - (daysOld * 86400);

    // Remove from buffer
    auto it = g_logBuffer.begin();
    while (it != g_logBuffer.end()) {
        if (it->timestamp < cutoffTime) {
            it = g_logBuffer.erase(it);
        } else {
            ++it;
        }
    }

    DBG_INFO("AuditLogger", "Old logs cleared (>" + String(daysOld) + " days)");
}

String Logger::formatTimestamp(uint32_t timestamp) {
    time_t t = timestamp;
    struct tm *timeinfo = localtime(&t);

    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return String(buffer);
}

void Logger::printLogSummary() {
    Serial.println();
    Serial.print(COLOR_CYAN);
    Serial.println("╔═══════════════════════════════════════╗");
    Serial.print("║ 📊 Audit Log Summary");
    Serial.println(String(19, ' ') + "║");
    Serial.println("╠═══════════════════════════════════════╣");

    uint32_t errorCount = 0, warnCount = 0, infoCount = 0, successCount = 0;

    for (const auto &entry : g_logBuffer) {
        if (entry.level == "ERROR") errorCount++;
        else if (entry.level == "WARN") warnCount++;
        else if (entry.level == "INFO") infoCount++;
        else if (entry.level == "SUCCESS") successCount++;
    }

    Serial.print("║ Total Entries: ");
    Serial.print(g_logBuffer.size());
    Serial.println(String(19, ' ') + "║");

    Serial.print("║ ✓ Success: ");
    Serial.print(COLOR_GREEN);
    Serial.print(successCount);
    Serial.print(COLOR_CYAN);
    Serial.println(String(24 - String(successCount).length(), ' ') + "║");

    Serial.print("║ ℹ Info: ");
    Serial.print(COLOR_BLUE);
    Serial.print(infoCount);
    Serial.print(COLOR_CYAN);
    Serial.println(String(26 - String(infoCount).length(), ' ') + "║");

    Serial.print("║ ⚠ Warning: ");
    Serial.print(COLOR_YELLOW);
    Serial.print(warnCount);
    Serial.print(COLOR_CYAN);
    Serial.println(String(24 - String(warnCount).length(), ' ') + "║");

    Serial.print("║ ✗ Error: ");
    Serial.print(COLOR_RED);
    Serial.print(errorCount);
    Serial.print(COLOR_CYAN);
    Serial.println(String(25 - String(errorCount).length(), ' ') + "║");

    Serial.println("╚═══════════════════════════════════════╝");
    Serial.print(COLOR_RESET);
}

void Logger::deleteAllLogs() {
    g_logBuffer.clear();
    g_auditResults.clear();

    LittleFS.remove(AUDIT_LOG_FILE);
    LittleFS.remove(AUDIT_RESULTS_FILE);

    DBG_WARN("AuditLogger", "All logs deleted");
}

}  // namespace AuditLogger
