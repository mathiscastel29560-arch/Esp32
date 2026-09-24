#ifndef OPERATION_LOGGER_H
#define OPERATION_LOGGER_H

#include <Arduino.h>
#include "high_speed_log_buffer.h"
#include "audit_log.h"
#include "tool_result_persistence.h"

// Unified logging helper for complex operations
// Provides audit logging + high-speed buffering + persistence in one call
class OperationLogger {
public:
    // Log operation start
    static void start(const char* tool_name, const char* operation) {
        AuditLog::instance().logToolStart(tool_name, operation);
    }

    // Fast event logging (goes to PSRAM buffer for intensive ops)
    static void logEvent(const char* tool_name, const char* event) {
        if (HighSpeedLogBuffer::instance().logFast(tool_name, event)) {
            // Logged to PSRAM
        } else {
            // Fallback to audit log
            AuditLog::instance().log(AuditEventType::TOOL_START, tool_name, event);
        }
    }

    // Log multiple fast events (for packet-based operations)
    static void logEvents(const char* tool_name, const char* events[], uint32_t count) {
        for (uint32_t i = 0; i < count; i++) {
            logEvent(tool_name, events[i]);
        }
    }

    // Log operation completion with persistence
    static void complete(const char* tool_name, const char* result_json) {
        AuditLog::instance().logToolStop(tool_name, true, "completed");
        ToolResultPersistence::instance().storeToolResult(tool_name, result_json);

        // Flush any buffered events
        HighSpeedLogBuffer::instance().flush();
    }

    // Log operation failure
    static void failed(const char* tool_name, const char* reason) {
        AuditLog::instance().logToolStop(tool_name, false, reason);
        AuditLog::instance().logError(tool_name, reason);
        HighSpeedLogBuffer::instance().flush();
    }

    // Initialize buffering for intensive operation
    static bool initBuffer(uint32_t size_kb = 256) {
        return HighSpeedLogBuffer::instance().begin(size_kb);
    }

    // Get buffer stats
    static HighSpeedLogBuffer::Stats getBufferStats() {
        return HighSpeedLogBuffer::instance().getStats();
    }

private:
    OperationLogger() {}
};

#endif
