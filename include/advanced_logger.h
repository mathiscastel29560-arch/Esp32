#pragma once
#include <Arduino.h>
#include <vector>

namespace AdvancedLogger {

enum LogLevel {
    LOG_VERBOSE = 0,
    LOG_DEBUG = 1,
    LOG_INFO = 2,
    LOG_WARNING = 3,
    LOG_ERROR = 4,
    LOG_CRITICAL = 5
};

enum LogTarget {
    LOG_SERIAL = 1,
    LOG_FILE = 2,
    LOG_MEMORY = 4,
    LOG_ALL = 7
};

struct LogEntry {
    uint32_t timestamp;
    LogLevel level;
    const char* tag;
    const char* message;
    const char* file;
    uint32_t line;
};

struct LogStats {
    uint32_t totalEntries;
    uint32_t verboseCount;
    uint32_t debugCount;
    uint32_t infoCount;
    uint32_t warningCount;
    uint32_t errorCount;
    uint32_t criticalCount;
};

void initLogger(uint8_t targets, LogLevel minLevel, uint32_t bufferSize = 10000);
void log(LogLevel level, const char* tag, const char* message, const char* file = "", uint32_t line = 0);
void logf(LogLevel level, const char* tag, const char* format, ...);

void setLogLevel(LogLevel level);
void setLogTargets(uint8_t targets);
void flushLogs();
void clearLogs();

std::vector<LogEntry> getLogHistory(uint32_t maxEntries = 100);
LogStats getLogStatistics();
void displayLogSummary();
void exportLogs(const char* filename);

} // namespace AdvancedLogger
