#ifndef LOG_DATABASE_H
#define LOG_DATABASE_H

#include <Arduino.h>
#include <LittleFS.h>

// SQLite-like structured logging database
// Stores logs with schema, supports querying and analysis

class LogDatabase {
public:
    static LogDatabase& instance() {
        static LogDatabase ldb;
        return ldb;
    }

    // Log entry structure (matches database schema)
    struct LogEntry {
        uint32_t timestamp;      // Unix timestamp
        uint8_t event_type;      // AuditEventType enum value
        char module[32];         // Module name
        char details[256];       // Event details
        uint32_t free_heap;      // Free heap at time of log
        uint8_t battery_percent; // Battery level
    };

    // Query filter
    struct QueryFilter {
        uint8_t event_type;      // -1 = any
        const char* module;      // nullptr = any
        uint32_t timestamp_from; // 0 = any
        uint32_t timestamp_to;   // 0 = any
    };

    // Initialize database
    bool begin(const char* db_path = "/logs/audit.db");

    // Insert log entry
    bool insertLog(const LogEntry& entry);

    // Query logs (returns JSON array)
    String query(const QueryFilter& filter, uint32_t limit = 100);

    // Get log count matching filter
    uint32_t count(const QueryFilter& filter);

    // Get log entry by index (0-based)
    bool getLogAt(uint32_t index, LogEntry& entry);

    // Delete logs older than timestamp
    bool deleteOlderThan(uint32_t timestamp);

    // Export to CSV file
    bool exportToCSV(const char* filepath, const QueryFilter& filter);

    // Export to JSON file
    bool exportToJSON(const char* filepath, const QueryFilter& filter);

    // Get database statistics
    struct Stats {
        uint32_t total_entries;
        uint32_t total_size_bytes;
        uint32_t oldest_timestamp;
        uint32_t newest_timestamp;
        uint8_t integrity_check;  // 0=ok, >0=corrupted
    };
    Stats getStats() const;

    // Rebuild database (defragmentation)
    bool rebuild();

    // Clear all logs
    bool clear();

    // Check if database is corrupted
    bool isCorrupted() const { return corrupted_; }

private:
    LogDatabase() : corrupted_(false), entry_count_(0) {}

    bool corrupted_;
    uint32_t entry_count_;
    String db_path_;

    // Helper: read all entries from storage
    bool loadEntries();

    // Helper: save all entries to storage
    bool saveEntries();

    // In-memory cache of entries (limited size)
    static constexpr uint32_t MAX_CACHED_ENTRIES = 1000;
    LogEntry* entries_;
    uint32_t entries_capacity_;

    // Helper: validate entry
    bool validateEntry(const LogEntry& entry);

};

#endif
