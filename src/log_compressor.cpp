#include "log_compressor.h"

// Simple LZ4-like compression with run-length encoding for repetitive data
// Format:
//   0x00 = end marker
//   0x01-0x7F = literal block (length follows, up to 127 bytes)
//   0x80-0xFF = match block (offset/length in next byte)

uint16_t LogCompressor::compress(const uint8_t* input, uint16_t input_len,
                                 uint8_t* output, uint16_t output_size) {
    if (!input || !output || input_len == 0 || output_size < 10) {
        return 0;
    }

    uint16_t out_pos = 0;
    uint16_t in_pos = 0;

    while (in_pos < input_len && out_pos < output_size - 2) {
        // Try to find matching sequence
        uint16_t match_len = 0;
        uint16_t match_offset = 0;

        // Look back up to 255 bytes for matching patterns
        if (in_pos >= 1) {
            for (uint16_t back = 1; back <= (in_pos > 255 ? 255 : in_pos); back++) {
                uint16_t len = 0;
                while (len < MAX_MATCH &&
                       in_pos + len < input_len &&
                       input[in_pos + len] == input[in_pos - back + len]) {
                    len++;
                }

                if (len >= MIN_MATCH && len > match_len) {
                    match_len = len;
                    match_offset = back;
                }
            }
        }

        // Use match if found and beneficial
        if (match_len >= MIN_MATCH) {
            // Match block: 0x80 + compressed offset/length
            if (out_pos + 2 >= output_size) break;

            output[out_pos++] = 0x80 | (match_len - MIN_MATCH);
            output[out_pos++] = match_offset - 1;
            in_pos += match_len;
        } else {
            // Literal block: collect consecutive non-matching bytes
            uint16_t lit_len = 0;
            uint16_t lit_start = out_pos++;

            while (lit_len < 127 && in_pos < input_len && out_pos < output_size) {
                // Check if next bytes could match
                uint16_t next_match = 0;
                for (uint16_t back = 1; back <= (in_pos > 255 ? 255 : in_pos); back++) {
                    uint16_t len = 0;
                    while (len < MAX_MATCH &&
                           in_pos + len < input_len &&
                           input[in_pos + len] == input[in_pos - back + len]) {
                        len++;
                    }
                    if (len >= MIN_MATCH) {
                        next_match = len;
                        break;
                    }
                }

                // Stop if good match ahead
                if (next_match >= MIN_MATCH) break;

                output[out_pos++] = input[in_pos++];
                lit_len++;
            }

            // Write literal block header
            output[lit_start] = 0x01 | (lit_len - 1);
        }
    }

    // End marker
    if (out_pos < output_size) {
        output[out_pos++] = 0x00;
    }

    // Only use compression if we saved space
    if (out_pos >= input_len) {
        return 0; // Not worth compressing
    }

    return out_pos;
}

uint16_t LogCompressor::decompress(const uint8_t* input, uint16_t input_len,
                                   uint8_t* output, uint16_t output_size) {
    if (!input || !output || input_len == 0) {
        return 0;
    }

    uint16_t in_pos = 0;
    uint16_t out_pos = 0;

    while (in_pos < input_len && out_pos < output_size) {
        uint8_t block = input[in_pos++];

        if (block == 0x00) {
            // End marker
            break;
        } else if (block & 0x80) {
            // Match block: 0x80 + (length-MIN_MATCH)
            if (in_pos >= input_len) break;

            uint16_t match_len = (block & 0x7F) + MIN_MATCH;
            uint16_t match_offset = input[in_pos++] + 1;

            if (match_offset > out_pos) {
                return 0; // Invalid offset
            }

            uint16_t match_pos = out_pos - match_offset;
            for (uint16_t i = 0; i < match_len && out_pos < output_size; i++) {
                output[out_pos++] = output[match_pos + i];
            }
        } else {
            // Literal block: 0x01 + (length-1)
            uint16_t lit_len = (block & 0x7F) + 1;

            if (in_pos + lit_len > input_len) break;
            if (out_pos + lit_len > output_size) break;

            memcpy(output + out_pos, input + in_pos, lit_len);
            out_pos += lit_len;
            in_pos += lit_len;
        }
    }

    return out_pos;
}
