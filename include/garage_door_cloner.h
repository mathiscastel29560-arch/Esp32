#ifndef GARAGE_DOOR_CLONER_H
#define GARAGE_DOOR_CLONER_H

#include <Arduino.h>

namespace GarageDoorCloner {
    struct GarageDoorCode {
        uint32_t timestamp;
        uint32_t frequency;
        uint32_t code;
        uint8_t length_bits;
        int8_t rssi;
    };

    bool begin();
    
    // Learn/capture a garage door code
    bool captureCode(uint32_t timeout_ms, GarageDoorCode& code);
    
    // Store captured code
    bool storeCode(const char* name, const GarageDoorCode& code);
    
    // Replay stored code
    bool replayCode(const GarageDoorCode& code, uint8_t repeat_count = 5);
    
    // Get stored codes list
    String listStoredCodes();
    
    struct Stats {
        uint32_t codes_captured;
        uint32_t codes_replayed;
        uint32_t success_rate;
    };
    Stats getStats();
}

#endif
