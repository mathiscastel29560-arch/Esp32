#include "advanced_logger.h"
#include <LittleFS.h>
#include <cstdarg>
#include <cstring>
#include <cstdio>

namespace AdvancedLogger {

static std::vector<LogEntry> logBuffer;
static uint32_t maxBufferSize = 10000;
static LogLevel minLogLevel = LOG_DEBUG;
static uint8_t logTargets = LOG_SERIAL | LOG_FILE;
static LogStats stats = {0, 0, 0, 0, 0, 0, 0};

const char* getLevelName(LogLevel level) {
    switch (level) {
        case LOG_VERBOSE: return "VERBOSE";
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO: return "INFO";
        case LOG_WARNING: return "WARNING";
        case LOG_ERROR: return "ERROR";
        case LOG_CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

int getLevelColor(LogLevel level) {
    switch (level) {
        case LOG_VERBOSE: return 36;  // Cyan
        case LOG_DEBUG: return 32;    // Green
        case LOG_INFO: return 34;     // Blue
        case LOG_WARNING: return 33;  // Yellow
        case LOG_ERROR: return 31;    // Red
        case LOG_CRITICAL: return 35; // Magenta
        default: return 37;           // White
    }
}

void initLogger(uint8_t targets, LogLevel minLevel, uint32_t bufferSize) {
    logTargets = targets;
    minLogLevel = minLevel;
    maxBufferSize = bufferSize;
    logBuffer.reserve(bufferSize);
    
    if (logTargets & LOG_FILE) {
        LittleFS.begin();
        LittleFS.mkdir("/logs/events");
        LittleFS.end();
    }
}

void log(LogLevel level, const char* tag, const char* message, const char* file, uint32_t line) {
    if (level < minLogLevel) return;
    
    LogEntry entry{millis(), level, tag, message, file, line};
    
    if (logTargets & LOG_MEMORY) {
        if (logBuffer.size() >= maxBufferSize) {
            logBuffer.erase(logBuffer.begin());
        }
        logBuffer.push_back(entry);
    }
    
    if (logTargets & LOG_SERIAL) {
        int color = getLevelColor(level);
        Serial.printf("\033[%dm[%s] %s: %s\033[0m", color, getLevelName(level), tag, message);
        if (file && line > 0) {
            Serial.printf(" (%s:%u)", file, line);
        }
        Serial.println();
    }
    
    if (logTargets & LOG_FILE) {
        if (LittleFS.begin()) {
            fs::File file = LittleFS.open("/logs/events/app.log", "a");
            if (file) {
                file.printf("[%u] [%s] %s: %s\n", millis(), getLevelName(level), tag, message);
                file.close();
            }
            LittleFS.end();
        }
    }
    
    switch (level) {
        case LOG_VERBOSE: stats.verboseCount++; break;
        case LOG_DEBUG: stats.debugCount++; break;
        case LOG_INFO: stats.infoCount++; break;
        case LOG_WARNING: stats.warningCount++; break;
        case LOG_ERROR: stats.errorCount++; break;
        case LOG_CRITICAL: stats.criticalCount++; break;
        default: break;
    }
    
    stats.totalEntries++;
}

void logf(LogLevel level, const char* tag, const char* format, ...) {
    if (level < minLogLevel) return;
    
    static char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    log(level, tag, buffer);
}

void setLogLevel(LogLevel level) {
    minLogLevel = level;
}

void setLogTargets(uint8_t targets) {
    logTargets = targets;
}

void flushLogs() {
    if (logTargets & LOG_FILE && LittleFS.begin()) {
        fs::File file = LittleFS.open("/logs/events/app.log", "a");
        if (file) {
            file.println("--- Log Flush ---");
            file.close();
        }
        LittleFS.end();
    }
}

void clearLogs() {
    logBuffer.clear();
    stats = {0, 0, 0, 0, 0, 0, 0};
    
    if (logTargets & LOG_FILE && LittleFS.begin()) {
        LittleFS.remove("/logs/events/app.log");
        LittleFS.end();
    }
}

std::vector<LogEntry> getLogHistory(uint32_t maxEntries) {
    std::vector<LogEntry> result;
    uint32_t startIdx = (logBuffer.size() > maxEntries) ? (logBuffer.size() - maxEntries) : 0;
    
    for (uint32_t i = startIdx; i < logBuffer.size(); i++) {
        result.push_back(logBuffer[i]);
    }
    
    return result;
}

LogStats getLogStatistics() {
    return stats;
}

void displayLogSummary() {
    Serial.println("\n╔════════════════════════════════════════════════════════════╗");
    Serial.println("║              LOGGING STATISTICS                            ║");
    Serial.println("╠════════════════════════════════════════════════════════════╣");
    
    Serial.printf("║ Total Entries: %u\n", stats.totalEntries);
    Serial.printf("║ Verbose: %u | Debug: %u | Info: %u\n", stats.verboseCount, stats.debugCount, stats.infoCount);
    Serial.printf("║ Warnings: %u | Errors: %u | Critical: %u\n", stats.warningCount, stats.errorCount, stats.criticalCount);
    Serial.printf("║ Buffer Usage: %u / %u entries\n", (uint32_t)logBuffer.size(), maxBufferSize);
    
    if (stats.errorCount > 0 || stats.criticalCount > 0) {
        Serial.printf("║ ⚠️  Issues detected: %u warnings, %u errors, %u critical\n",
                     stats.warningCount, stats.errorCount, stats.criticalCount);
    } else {
        Serial.println("║ ✓ System operating normally");
    }
    
    Serial.println("╚════════════════════════════════════════════════════════════╝\n");
}

void exportLogs(const char* filename) {
    if (!LittleFS.begin()) return;
    
    LittleFS.mkdir("/logs/exports");
    
    fs::File outFile = LittleFS.open(filename, "w");
    if (!outFile) {
        LittleFS.end();
        return;
    }
    
    outFile.println("Timestamp,Level,Tag,Message");
    
    for (const auto& entry : logBuffer) {
        outFile.printf("%u,%s,%s,%s\n", entry.timestamp, getLevelName(entry.level), 
                      entry.tag, entry.message);
    }
    
    outFile.close();
    LittleFS.end();
}

} // namespace AdvancedLogger
