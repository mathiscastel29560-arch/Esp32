#ifndef PSRAM_LOG_BUFFER_H
#define PSRAM_LOG_BUFFER_H

#include <Arduino.h>
#include <cstring>

// Circular buffer in PSRAM for unlimited logging
// Automatically overwrites oldest entries when full

class PSRAMLogBuffer {
public:
    static PSRAMLogBuffer& instance() {
        static PSRAMLogBuffer lb;
        return lb;
    }

    // Initialize with size in KB (e.g., 4096 = 4MB)
    bool begin(uint32_t size_kb);
    
    // Write log entry (auto-rotates if full)
    bool write(const char* entry);
    
    // Read from buffer
    const char* read(uint32_t index);
    
    // Get entry count
    uint32_t getEntryCount() const { return entry_count_; }
    
    // Get used percentage
    uint8_t getUsagePercent() const;
    
    // Export all to file
    bool exportToFile(const char* filepath);
    
    // Clear all
    void clear();
    
    // Get stats
    struct Stats {
        uint32_t total_size;
        uint32_t used_size;
        uint32_t entry_count;
        uint32_t overflow_count;
        float avg_entry_size;
    };
    
    Stats getStats() const;

private:
    PSRAMLogBuffer();
    ~PSRAMLogBuffer();
    
    uint8_t* buffer_;
    uint32_t buffer_size_;
    uint32_t write_pos_;
    uint32_t entry_count_;
    uint32_t overflow_count_;
    
    struct EntryHeader {
        uint32_t timestamp;
        uint16_t size;
        uint8_t type;  // 0=log, 1=attack, 2=error
    };
    
    // Helper: find next entry position
    uint32_t findNextEntry(uint32_t from);
    
    // Helper: check if space available
    bool ensureSpace(uint32_t needed);
};

#endif
