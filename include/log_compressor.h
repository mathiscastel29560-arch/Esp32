#ifndef LOG_COMPRESSOR_H
#define LOG_COMPRESSOR_H

#include <Arduino.h>
#include <cstring>

// LZ4-inspired simple compression for log entries
// Reduces PSRAM usage by 40-60% depending on log content

class LogCompressor {
public:
    // Compress data - returns compressed size, or 0 on failure
    static uint16_t compress(const uint8_t* input, uint16_t input_len,
                             uint8_t* output, uint16_t output_size);

    // Decompress data - returns decompressed size, or 0 on failure
    static uint16_t decompress(const uint8_t* input, uint16_t input_len,
                               uint8_t* output, uint16_t output_size);

    // Check if compression is worth it for this size
    static bool worthCompressing(uint16_t size) {
        return size > 100; // Compress if > 100 bytes
    }

    // Get compression ratio estimate
    static uint8_t getEstimatedRatio() {
        return 60; // Typically achieve 60% size (40% savings)
    }

private:
    // Internal compression with simple run-length and literal blocks
    static constexpr uint16_t MIN_MATCH = 4;
    static constexpr uint16_t MAX_MATCH = 128;
    static constexpr uint16_t BLOCK_SIZE = 256;

    // Literal block: 1 byte marker + up to 255 literal bytes
    // Match block: 2 bytes (offset, length)
};

#endif
