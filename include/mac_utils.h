#pragma once
#include <Arduino.h>

namespace MacUtils {

inline String toString(const uint8_t mac[6]) {
    char buf[18];
    snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(buf);
}

inline bool parse(const String &str, uint8_t out[6]) {
    uint8_t vals[6];
    int idx = 0;
    int pos = 0;
    while (idx < 6 && pos < (int)str.length()) {
        String byteStr = str.substring(pos, pos + 2);
        if (byteStr.length() < 2) return false;

        char *endPtr = nullptr;
        long val = strtoul(byteStr.c_str(), &endPtr, 16);

        if (endPtr == byteStr.c_str() || val < 0 || val > 255) {
            return false;
        }

        vals[idx++] = (uint8_t)val;
        pos += 3; // skip 2 hex chars + 1 separator
    }
    if (idx != 6) return false;
    memcpy(out, vals, 6);
    return true;
}

static const uint8_t BROADCAST[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

} // namespace MacUtils
