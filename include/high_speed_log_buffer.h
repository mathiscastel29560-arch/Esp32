#ifndef HIGH_SPEED_LOG_BUFFER_H
#define HIGH_SPEED_LOG_BUFFER_H

#include <Arduino.h>
#include <LittleFS.h>
#include "tool_result_persistence.h"

// High-speed circular buffer in PSRAM for intensive logging operations
// Reduces I/O bottleneck during heavy scanning/jamming/attack operations
// Automatically flushes to persistent storage when buffer fills or session ends

class HighSpeedLogBuffer {
public:
    static HighSpeedLogBuffer& instance() {
        static HighSpeedLogBuffer buf;
        return buf;
    }

    // Initialize buffer in PSRAM (size in KB, default 256KB)
    bool begin(uint32_t size_kb = 256) {
        if (heap_caps_get_free_size(MALLOC_CAP_SPIRAM) < (size_kb * 1024)) {
            Serial.println("[HighSpeedLogBuffer] Insufficient PSRAM available");
            return false;
        }

        buffer_size_ = size_kb * 1024;
        buffer_ = (uint8_t*)heap_caps_malloc(buffer_size_, MALLOC_CAP_SPIRAM);

        if (!buffer_) {
            Serial.println("[HighSpeedLogBuffer] Failed to allocate PSRAM buffer");
            return false;
        }

        write_pos_ = 0;
        entry_count_ = 0;
        enabled_ = true;

        Serial.printf("[HighSpeedLogBuffer] Initialized: %u KB\n", size_kb);
        return true;
    }

    // Log entry quickly to PSRAM buffer (non-blocking)
    bool logFast(const char* tool, const char* data) {
        if (!enabled_ || !buffer_) return false;

        // Format: [timestamp:entry_size:tool:data\n]
        String entry = String("[") + String(millis()) + ":" + String(tool) + ":" + String(data) + "]\n";
        uint16_t entry_len = entry.length();

        // Check if buffer is full
        if ((write_pos_ + entry_len) > buffer_size_) {
            // Buffer full - flush to disk
            flush();
            write_pos_ = 0;
        }

        // Copy entry to PSRAM
        memcpy(buffer_ + write_pos_, entry.c_str(), entry_len);
        write_pos_ += entry_len;
        entry_count_++;

        return true;
    }

    // Get current usage percentage
    uint8_t getUsagePercent() const {
        if (!buffer_) return 0;
        return (write_pos_ * 100) / buffer_size_;
    }

    // Get entry count
    uint32_t getEntryCount() const {
        return entry_count_;
    }

    // Manual flush to persistent storage
    bool flush() {
        if (!buffer_ || write_pos_ == 0) return true;

        // Create timestamped result file
        String timestamp = String(millis() / 1000);
        String filename = "/results/buffer_" + timestamp + ".log";

        fs::File file = LittleFS.open(filename, "w");
        if (!file) {
            Serial.printf("[HighSpeedLogBuffer] Failed to open %s\n", filename.c_str());
            return false;
        }

        // Write buffer contents to file
        size_t written = file.write(buffer_, write_pos_);
        file.close();

        if (written == write_pos_) {
            Serial.printf("[HighSpeedLogBuffer] Flushed %u bytes to %s (%u entries)\n",
                         written, filename.c_str(), entry_count_);
            return true;
        }

        return false;
    }

    // Clear buffer without flushing
    void clear() {
        write_pos_ = 0;
        entry_count_ = 0;
    }

    // Disable/enable buffer (for critical sections)
    void setEnabled(bool enabled) {
        enabled_ = enabled;
    }

    // Shutdown and cleanup
    void end() {
        if (buffer_) {
            flush();
            heap_caps_free(buffer_);
            buffer_ = nullptr;
            enabled_ = false;
            Serial.println("[HighSpeedLogBuffer] Shutdown complete");
        }
    }

    // Get stats
    struct Stats {
        uint32_t buffer_size;
        uint32_t used_size;
        uint32_t entry_count;
        uint8_t usage_percent;
        bool enabled;
    };

    Stats getStats() const {
        return {
            buffer_size_,
            write_pos_,
            entry_count_,
            getUsagePercent(),
            enabled_
        };
    }

private:
    HighSpeedLogBuffer()
        : buffer_(nullptr), buffer_size_(0), write_pos_(0),
          entry_count_(0), enabled_(false) {}

    uint8_t* buffer_;
    uint32_t buffer_size_;
    uint32_t write_pos_;
    uint32_t entry_count_;
    bool enabled_;
};

#endif
