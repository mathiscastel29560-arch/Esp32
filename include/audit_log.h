#ifndef AUDIT_LOG_H
#define AUDIT_LOG_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ctime>

enum class AuditEventType {
    TOOL_START = 1,
    TOOL_STOP = 2,
    TOOL_SUCCESS = 3,
    TOOL_FAILURE = 4,
    DEVICE_FOUND = 5,
    ATTACK_INITIATED = 6,
    ATTACK_COMPLETED = 7,
    CONFIG_CHANGED = 8,
    ERROR_OCCURRED = 9,
    TX_ARMED = 10,
    TX_DISARMED = 11
};

class AuditLog {
public:
    static AuditLog& instance() {
        static AuditLog al;
        return al;
    }

    // Initialize audit logging
    bool begin() {
        if (!LittleFS.begin()) {
            Serial.println("[AuditLog] Failed to mount LittleFS");
            return false;
        }

        if (!LittleFS.exists("/logs")) {
            LittleFS.mkdir("/logs");
        }
        if (!LittleFS.exists("/logs/audit")) {
            LittleFS.mkdir("/logs/audit");
        }

        Serial.println("[AuditLog] Initialized");
        return true;
    }

    // Log event with details
    void log(AuditEventType type, const char* module, const char* details = "") {
        unsigned long now = millis();
        uint32_t free_heap = esp_get_free_heap_size();

        // Format: timestamp,event_type,module,free_heap,details
        String event = String(now) + "," +
                      String((uint8_t)type) + "," +
                      String(module) + "," +
                      String(free_heap) + "," +
                      String(details);

        // Write to daily log file
        String filename = "/logs/audit/audit_" + String(getDay()) + ".csv";
        File file = LittleFS.open(filename, "a");

        if (file) {
            file.println(event);
            file.close();

            // Also print to serial for real-time monitoring
            char type_str[20];
            getEventTypeString(type, type_str, sizeof(type_str));
            Serial.printf("[AUDIT] %s | %s | %s\n", type_str, module, details);
        } else {
            Serial.printf("[AuditLog] Failed to open audit log: %s\n", filename.c_str());
        }
    }

    // Log tool execution
    void logToolStart(const char* toolName, const char* parameters) {
        String details = "params=";
        details += parameters;
        log(AuditEventType::TOOL_START, toolName, details.c_str());
    }

    void logToolStop(const char* toolName, bool success, const char* result) {
        String details = success ? "success" : "failed";
        details += ",result=";
        details += result;
        log(success ? AuditEventType::TOOL_SUCCESS : AuditEventType::TOOL_FAILURE,
            toolName, details.c_str());
    }

    // Log device discovery
    void logDeviceFound(const char* toolName, const char* device_info) {
        String details = "device=";
        details += device_info;
        log(AuditEventType::DEVICE_FOUND, toolName, details.c_str());
    }

    // Log attack action
    void logAttack(const char* toolName, const char* target, bool initiated) {
        String details = "target=";
        details += target;
        log(initiated ? AuditEventType::ATTACK_INITIATED : AuditEventType::ATTACK_COMPLETED,
            toolName, details.c_str());
    }

    // Log configuration change
    void logConfigChange(const char* toolName, const char* parameter, const char* old_value, const char* new_value) {
        String details = "";
        details += parameter;
        details += ":";
        details += old_value;
        details += "->";
        details += new_value;
        log(AuditEventType::CONFIG_CHANGED, toolName, details.c_str());
    }

    // Log error
    void logError(const char* module, const char* error_msg) {
        String details = "error=";
        details += error_msg;
        log(AuditEventType::ERROR_OCCURRED, module, details.c_str());
    }

    // Get audit report
    void printReport(uint8_t day = 0) {
        String filename = "/logs/audit/audit_";
        filename += (day == 0) ? String(getDay()) : String(day);
        filename += ".csv";

        if (!LittleFS.exists(filename)) {
            Serial.printf("[AuditLog] No audit log found for day %d\n", day == 0 ? getDay() : day);
            return;
        }

        Serial.println("\n========== AUDIT REPORT ==========");
        Serial.println("Time,Type,Module,FreeHeap,Details");

        File file = LittleFS.open(filename, "r");
        while (file.available()) {
            String line = file.readStringUntil('\n');
            Serial.println(line);
        }
        file.close();

        Serial.println("==================================\n");
    }

    // List audit files
    void listAuditFiles() {
        File root = LittleFS.open("/logs/audit");
        if (!root) {
            Serial.println("[AuditLog] Audit directory not found");
            return;
        }

        Serial.println("[AuditLog] Audit files:");
        File file = root.openNextFile();
        uint32_t total_size = 0;
        while (file) {
            if (!file.isDirectory()) {
                uint32_t size = file.size();
                total_size += size;
                Serial.printf("  - %s (%u bytes, %u lines)\n",
                             file.name(), size, size / 50); // Rough line count
            }
            file = root.openNextFile();
        }

        Serial.printf("[AuditLog] Total audit data: %u bytes\n", total_size);
    }

    // Clear old audit logs (keep last N days)
    void clearOldLogs(uint8_t keep_days = 7) {
        File root = LittleFS.open("/logs/audit");
        if (!root) return;

        uint8_t today = getDay();
        uint8_t count = 0;

        File file = root.openNextFile();
        while (file) {
            String name = file.name();
            // Parse day from filename "audit_N.csv"
            uint8_t file_day = atoi(name.c_str() + 6); // Skip "audit_"

            if ((today - file_day) > keep_days) {
                String filepath = "/logs/audit/" + name;
                if (LittleFS.remove(filepath)) {
                    count++;
                }
            }
            file = root.openNextFile();
        }

        if (count > 0) {
            Serial.printf("[AuditLog] Cleaned up %u old audit files\n", count);
        }
    }

private:
    uint8_t getDay() {
        // Simple day counter - increments every 24 hours of uptime
        return (millis() / (24 * 60 * 60 * 1000)) % 256;
    }

    void getEventTypeString(AuditEventType type, char* buffer, size_t size) {
        const char* type_str = "UNKNOWN";
        switch(type) {
            case AuditEventType::TOOL_START: type_str = "TOOL_START"; break;
            case AuditEventType::TOOL_STOP: type_str = "TOOL_STOP"; break;
            case AuditEventType::TOOL_SUCCESS: type_str = "SUCCESS"; break;
            case AuditEventType::TOOL_FAILURE: type_str = "FAILURE"; break;
            case AuditEventType::DEVICE_FOUND: type_str = "DEVICE_FOUND"; break;
            case AuditEventType::ATTACK_INITIATED: type_str = "ATTACK_START"; break;
            case AuditEventType::ATTACK_COMPLETED: type_str = "ATTACK_END"; break;
            case AuditEventType::CONFIG_CHANGED: type_str = "CONFIG_CHANGED"; break;
            case AuditEventType::ERROR_OCCURRED: type_str = "ERROR"; break;
            case AuditEventType::TX_ARMED: type_str = "TX_ARMED"; break;
            case AuditEventType::TX_DISARMED: type_str = "TX_DISARMED"; break;
        }
        strncpy(buffer, type_str, size - 1);
        buffer[size - 1] = '\0';
    }
};

#define AUDIT_LOG(type, module, details) AuditLog::instance().log(type, module, details)
#define AUDIT_TOOL_START(tool, params) AuditLog::instance().logToolStart(tool, params)
#define AUDIT_TOOL_STOP(tool, success, result) AuditLog::instance().logToolStop(tool, success, result)
#define AUDIT_ERROR(module, error) AuditLog::instance().logError(module, error)

#endif
