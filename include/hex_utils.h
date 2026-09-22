#pragma once
#include <Arduino.h>
#include <cstdint>

namespace HexUtils {

// Convert byte array to hex string efficiently
// e.g. {0xAB, 0xCD} -> "ABCD"
inline String toHexString(const uint8_t* data, uint32_t len) {
    if (!data || len == 0) return "";

    char hexStr[len * 2 + 1];
    for (uint32_t i = 0; i < len; i++) {
        snprintf(&hexStr[i * 2], 3, "%02X", data[i]);
    }
    hexStr[len * 2] = '\0';
    return String(hexStr);
}

// Convert single byte to hex string
inline String byteToHex(uint8_t byte) {
    char buf[3];
    snprintf(buf, sizeof(buf), "%02X", byte);
    return String(buf);
}

}  // namespace HexUtils
