#include "psram_log_buffer.h"
#include "log_compressor.h"
#include <LittleFS.h>
#include <esp_heap_caps.h>

PSRAMLogBuffer::PSRAMLogBuffer()
    : buffer_(nullptr), buffer_size_(0), write_pos_(0),
      entry_count_(0), overflow_count_(0), compression_enabled_(true) {}

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
    if (entry_len > 65535) entry_len = 65535;

    // Try compression if enabled
    uint8_t* data_to_write = (uint8_t*)entry;
    uint16_t data_len = entry_len;
    bool is_compressed = false;
    uint8_t* compressed_buf = nullptr;

    if (compression_enabled_ && LogCompressor::worthCompressing(entry_len)) {
        // Allocate temporary buffer for compression
        compressed_buf = (uint8_t*)malloc(entry_len);
        if (compressed_buf) {
            uint16_t compressed_len = LogCompressor::compress(
                (uint8_t*)entry, entry_len,
                compressed_buf, entry_len);

            // Use compression if it saved space
            if (compressed_len > 0 && compressed_len < entry_len) {
                data_to_write = compressed_buf;
                data_len = compressed_len;
                is_compressed = true;
            }
        }
    }

    EntryHeader hdr;
    hdr.timestamp = millis();
    hdr.size = data_len;
    hdr.type = is_compressed ? 1 : 0;  // 0=uncompressed, 1=compressed

    uint32_t needed = sizeof(EntryHeader) + data_len;

    // Check if we need to wrap around
    if (write_pos_ + needed > buffer_size_) {
        write_pos_ = 0;
        overflow_count_++;
    }

    // Write header
    memcpy(buffer_ + write_pos_, &hdr, sizeof(EntryHeader));
    write_pos_ += sizeof(EntryHeader);

    // Write entry (compressed or original)
    memcpy(buffer_ + write_pos_, data_to_write, data_len);
    write_pos_ += data_len;

    if (compressed_buf) {
        free(compressed_buf);
    }

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
    uint8_t* data = buffer_ + pos + sizeof(EntryHeader);

    // If compressed, decompress and return
    if (hdr->type == 1) {  // Compressed
        static uint8_t decomp_buf[2048];
        uint16_t decomp_len = LogCompressor::decompress(data, hdr->size,
                                                        decomp_buf, sizeof(decomp_buf) - 1);
        if (decomp_len > 0) {
            decomp_buf[decomp_len] = '\0';  // Null terminate
            return (const char*)decomp_buf;
        }
        return nullptr;
    }

    return (const char*)data;
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
        uint8_t* data = buffer_ + pos + sizeof(EntryHeader);

        // Decompress if needed
        const char* entry_str = (const char*)data;
        if (hdr->type == 1) {  // Compressed
            static uint8_t decomp_buf[2048];
            uint16_t decomp_len = LogCompressor::decompress(data, hdr->size,
                                                           decomp_buf, sizeof(decomp_buf) - 1);
            if (decomp_len > 0) {
                decomp_buf[decomp_len] = '\0';
                entry_str = (const char*)decomp_buf;
            }
        }

        // Write CSV: timestamp,size,entry
        f.printf("%lu,%d,%s\n", hdr->timestamp, hdr->size, entry_str);

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
