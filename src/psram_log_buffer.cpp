#include "psram_log_buffer.h"
#include <LittleFS.h>
#include <esp_heap_caps.h>

PSRAMLogBuffer::PSRAMLogBuffer() 
    : buffer_(nullptr), buffer_size_(0), write_pos_(0), 
      entry_count_(0), overflow_count_(0) {}

PSRAMLogBuffer::~PSRAMLogBuffer() {
    if (buffer_) {
        heap_caps_free(buffer_);
    }
}

bool PSRAMLogBuffer::begin(uint32_t size_kb) {
    buffer_size_ = size_kb * 1024;
    
    // Allocate from PSRAM (caps_flags = MALLOC_CAP_SPIRAM)
    buffer_ = (uint8_t*)heap_caps_malloc(buffer_size_, MALLOC_CAP_SPIRAM);
    
    if (!buffer_) {
        Serial.printf("[PSRAMLogBuffer] Failed to allocate %d KB from PSRAM\n", size_kb);
        return false;
    }
    
    memset(buffer_, 0, buffer_size_);
    write_pos_ = 0;
    entry_count_ = 0;
    overflow_count_ = 0;
    
    Serial.printf("[PSRAMLogBuffer] Initialized: %dKB from PSRAM\n", size_kb);
    
    return true;
}

bool PSRAMLogBuffer::write(const char* entry) {
    if (!buffer_ || !entry) return false;
    
    uint16_t entry_len = strlen(entry);
    if (entry_len > 65535) entry_len = 65535;  // Max 64KB per entry
    
    EntryHeader hdr;
    hdr.timestamp = millis();
    hdr.size = entry_len;
    hdr.type = 0;  // log type
    
    uint32_t needed = sizeof(EntryHeader) + entry_len;
    
    // Check if we need to wrap around
    if (write_pos_ + needed > buffer_size_) {
        write_pos_ = 0;
        overflow_count_++;
    }
    
    // Write header
    memcpy(buffer_ + write_pos_, &hdr, sizeof(EntryHeader));
    write_pos_ += sizeof(EntryHeader);
    
    // Write entry
    memcpy(buffer_ + write_pos_, entry, entry_len);
    write_pos_ += entry_len;
    
    entry_count_++;
    
    return true;
}

const char* PSRAMLogBuffer::read(uint32_t index) {
    if (!buffer_ || index >= entry_count_) return nullptr;
    
    uint32_t pos = 0;
    uint32_t count = 0;
    
    while (pos < write_pos_ && count < index) {
        EntryHeader* hdr = (EntryHeader*)(buffer_ + pos);
        pos += sizeof(EntryHeader) + hdr->size;
        count++;
    }
    
    if (count != index) return nullptr;
    
    EntryHeader* hdr = (EntryHeader*)(buffer_ + pos);
    return (const char*)(buffer_ + pos + sizeof(EntryHeader));
}

uint8_t PSRAMLogBuffer::getUsagePercent() const {
    if (!buffer_) return 0;
    return (write_pos_ * 100) / buffer_size_;
}

bool PSRAMLogBuffer::exportToFile(const char* filepath) {
    if (!buffer_) return false;
    
    if (!LittleFS.begin()) return false;
    
    File f = LittleFS.open(filepath, FILE_WRITE);
    if (!f) {
        LittleFS.end();
        return false;
    }
    
    uint32_t pos = 0;
    uint32_t count = 0;
    
    while (pos < write_pos_) {
        EntryHeader* hdr = (EntryHeader*)(buffer_ + pos);
        
        // Write CSV: timestamp,size,entry
        f.printf("%lu,%d,%s\n", hdr->timestamp, hdr->size, 
                 (const char*)(buffer_ + pos + sizeof(EntryHeader)));
        
        pos += sizeof(EntryHeader) + hdr->size;
        count++;
    }
    
    f.close();
    LittleFS.end();
    
    Serial.printf("[PSRAMLogBuffer] Exported %d entries to %s\n", count, filepath);
    
    return true;
}

void PSRAMLogBuffer::clear() {
    if (!buffer_) return;
    memset(buffer_, 0, buffer_size_);
    write_pos_ = 0;
    entry_count_ = 0;
    overflow_count_ = 0;
}

PSRAMLogBuffer::Stats PSRAMLogBuffer::getStats() const {
    Stats s;
    s.total_size = buffer_size_;
    s.used_size = write_pos_;
    s.entry_count = entry_count_;
    s.overflow_count = overflow_count_;
    s.avg_entry_size = entry_count_ > 0 ? (float)write_pos_ / entry_count_ : 0;
    return s;
}
