#include "log_database.h"
#include <cstring>

bool LogDatabase::begin(const char* db_path) {
    if (!db_path) return false;

    db_path_ = db_path;

    // Allocate entry cache
    entries_ = (LogEntry*)malloc(sizeof(LogEntry) * MAX_CACHED_ENTRIES);
    if (!entries_) {
        Serial.println("[LogDatabase] Failed to allocate entry cache");
        return false;
    }

    entries_capacity_ = MAX_CACHED_ENTRIES;

    // Load existing entries from storage
    if (!loadEntries()) {
        entry_count_ = 0;
        corrupted_ = false; // First boot, not corrupted
    }

    Serial.printf("[LogDatabase] Initialized with %u entries\n", entry_count_);
    return true;
}

bool LogDatabase::insertLog(const LogEntry& entry) {
    if (!entries_ || entry_count_ >= entries_capacity_) {
        // Database full - shift entries and make room (ring buffer style)
        if (entry_count_ >= entries_capacity_) {
            memmove(entries_, entries_ + 1,
                   sizeof(LogEntry) * (entries_capacity_ - 1));
            entry_count_--;
        }
    }

    if (!validateEntry(entry)) {
        Serial.println("[LogDatabase] Invalid entry rejected");
        return false;
    }

    entries_[entry_count_] = entry;
    entry_count_++;

    // Periodically save to storage (every 10 entries or so)
    if (entry_count_ % 10 == 0) {
        saveEntries();
    }

    return true;
}

String LogDatabase::query(const QueryFilter& filter, uint32_t limit) {
    String result = "{\"results\":[";

    uint32_t count = 0;
    for (uint32_t i = 0; i < entry_count_ && count < limit; i++) {
        LogEntry& entry = entries_[i];

        // Apply filters
        if (filter.event_type != 0xFF && entry.event_type != filter.event_type) continue;
        if (filter.module && strcmp(entry.module, filter.module) != 0) continue;
        if (filter.timestamp_from > 0 && entry.timestamp < filter.timestamp_from) continue;
        if (filter.timestamp_to > 0 && entry.timestamp > filter.timestamp_to) continue;

        // Add to results
        if (count > 0) result += ",";
        result += "{";
        result += "\"timestamp\":" + String(entry.timestamp) + ",";
        result += "\"type\":" + String(entry.event_type) + ",";
        result += "\"module\":\"" + String(entry.module) + "\",";
        result += "\"details\":\"" + String(entry.details) + "\",";
        result += "\"heap\":" + String(entry.free_heap) + ",";
        result += "\"battery\":" + String(entry.battery_percent);
        result += "}";

        count++;
    }

    result += "],\"count\":" + String(count) + ",\"total\":" + String(entry_count_) + "}";
    return result;
}

uint32_t LogDatabase::count(const QueryFilter& filter) {
    uint32_t count = 0;

    for (uint32_t i = 0; i < entry_count_; i++) {
        LogEntry& entry = entries_[i];

        if (filter.event_type != 0xFF && entry.event_type != filter.event_type) continue;
        if (filter.module && strcmp(entry.module, filter.module) != 0) continue;
        if (filter.timestamp_from > 0 && entry.timestamp < filter.timestamp_from) continue;
        if (filter.timestamp_to > 0 && entry.timestamp > filter.timestamp_to) continue;

        count++;
    }

    return count;
}

bool LogDatabase::getLogAt(uint32_t index, LogEntry& entry) {
    if (index >= entry_count_) return false;

    entry = entries_[index];
    return true;
}

bool LogDatabase::deleteOlderThan(uint32_t timestamp) {
    uint32_t removed = 0;

    for (uint32_t i = 0; i < entry_count_; ) {
        if (entries_[i].timestamp < timestamp) {
            // Remove this entry by shifting
            memmove(entries_ + i, entries_ + i + 1,
                   sizeof(LogEntry) * (entry_count_ - i - 1));
            entry_count_--;
            removed++;
        } else {
            i++;
        }
    }

    if (removed > 0) {
        saveEntries();
        Serial.printf("[LogDatabase] Deleted %u old entries\n", removed);
    }

    return true;
}

bool LogDatabase::exportToCSV(const char* filepath, const QueryFilter& filter) {
    if (!LittleFS.begin()) return false;

    File f = LittleFS.open(filepath, FILE_WRITE);
    if (!f) {
        LittleFS.end();
        return false;
    }

    // Write header
    f.println("Timestamp,EventType,Module,Details,FreeHeap,Battery%");

    uint32_t count = 0;
    for (uint32_t i = 0; i < entry_count_; i++) {
        LogEntry& entry = entries_[i];

        // Apply filters
        if (filter.event_type != 0xFF && entry.event_type != filter.event_type) continue;
        if (filter.module && strcmp(entry.module, filter.module) != 0) continue;
        if (filter.timestamp_from > 0 && entry.timestamp < filter.timestamp_from) continue;
        if (filter.timestamp_to > 0 && entry.timestamp > filter.timestamp_to) continue;

        // Write row
        f.printf("%lu,%u,%s,%s,%lu,%u\n",
                entry.timestamp, entry.event_type, entry.module,
                entry.details, entry.free_heap, entry.battery_percent);
        count++;
    }

    f.close();
    LittleFS.end();

    Serial.printf("[LogDatabase] Exported %u entries to %s\n", count, filepath);
    return true;
}

bool LogDatabase::exportToJSON(const char* filepath, const QueryFilter& filter) {
    if (!LittleFS.begin()) return false;

    File f = LittleFS.open(filepath, FILE_WRITE);
    if (!f) {
        LittleFS.end();
        return false;
    }

    f.print("{\"entries\":[");

    uint32_t count = 0;
    for (uint32_t i = 0; i < entry_count_; i++) {
        LogEntry& entry = entries_[i];

        if (filter.event_type != 0xFF && entry.event_type != filter.event_type) continue;
        if (filter.module && strcmp(entry.module, filter.module) != 0) continue;
        if (filter.timestamp_from > 0 && entry.timestamp < filter.timestamp_from) continue;
        if (filter.timestamp_to > 0 && entry.timestamp > filter.timestamp_to) continue;

        if (count > 0) f.print(",");
        f.printf("{\"timestamp\":%lu,\"type\":%u,\"module\":\"%s\",\"details\":\"%s\",\"heap\":%lu,\"battery\":%u}",
                entry.timestamp, entry.event_type, entry.module,
                entry.details, entry.free_heap, entry.battery_percent);

        count++;
    }

    f.printf("],\"total\":%u}", count);
    f.close();
    LittleFS.end();

    Serial.printf("[LogDatabase] Exported %u entries to JSON\n", count);
    return true;
}

LogDatabase::Stats LogDatabase::getStats() const {
    Stats s = {0};
    s.total_entries = entry_count_;
    s.total_size_bytes = sizeof(LogEntry) * entry_count_;
    s.integrity_check = corrupted_ ? 1 : 0;

    if (entry_count_ > 0) {
        s.oldest_timestamp = entries_[0].timestamp;
        s.newest_timestamp = entries_[entry_count_ - 1].timestamp;
    }

    return s;
}

bool LogDatabase::rebuild() {
    Serial.println("[LogDatabase] Starting database rebuild...");

    // Save current entries
    LogEntry* backup = (LogEntry*)malloc(sizeof(LogEntry) * entry_count_);
    if (!backup) return false;

    memcpy(backup, entries_, sizeof(LogEntry) * entry_count_);

    // Clear
    memset(entries_, 0, sizeof(LogEntry) * entries_capacity_);
    entry_count_ = 0;

    // Restore valid entries only
    for (uint32_t i = 0; i < entry_count_; i++) {
        if (validateEntry(backup[i])) {
            insertLog(backup[i]);
        }
    }

    free(backup);
    corrupted_ = false;

    Serial.printf("[LogDatabase] Rebuild complete: %u valid entries\n", entry_count_);
    return true;
}

bool LogDatabase::clear() {
    memset(entries_, 0, sizeof(LogEntry) * entries_capacity_);
    entry_count_ = 0;
    saveEntries();
    Serial.println("[LogDatabase] Cleared all entries");
    return true;
}

bool LogDatabase::loadEntries() {
    if (!LittleFS.begin()) return false;

    if (!LittleFS.exists(db_path_.c_str())) {
        LittleFS.end();
        return true; // First boot, OK
    }

    File f = LittleFS.open(db_path_.c_str(), "r");
    if (!f) {
        LittleFS.end();
        return false;
    }

    entry_count_ = 0;
    while (f.available() && entry_count_ < entries_capacity_) {
        if (f.read((uint8_t*)&entries_[entry_count_], sizeof(LogEntry)) != sizeof(LogEntry)) {
            corrupted_ = true;
            break;
        }
        entry_count_++;
    }

    f.close();
    LittleFS.end();

    Serial.printf("[LogDatabase] Loaded %u entries from storage\n", entry_count_);
    return true;
}

bool LogDatabase::saveEntries() {
    if (!LittleFS.begin()) return false;

    // Create backup first
    if (LittleFS.exists(db_path_.c_str())) {
        LittleFS.rename(db_path_.c_str(), (db_path_ + ".bak").c_str());
    }

    File f = LittleFS.open(db_path_.c_str(), FILE_WRITE);
    if (!f) {
        LittleFS.end();
        return false;
    }

    for (uint32_t i = 0; i < entry_count_; i++) {
        if (f.write((uint8_t*)&entries_[i], sizeof(LogEntry)) != sizeof(LogEntry)) {
            f.close();
            LittleFS.end();
            return false;
        }
    }

    f.close();

    // Remove backup on success
    LittleFS.remove((db_path_ + ".bak").c_str());
    LittleFS.end();

    return true;
}

bool LogDatabase::validateEntry(const LogEntry& entry) {
    // Basic validation
    if (entry.event_type > 11) return false;  // Invalid event type
    if (strlen(entry.module) == 0) return false;
    if (entry.timestamp == 0) return false;

    return true;
}
