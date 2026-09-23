#ifndef BLE_JAMMER_H
#define BLE_JAMMER_H

#include <Arduino.h>

namespace BleJammer {
    bool begin();
    bool startJamming(uint32_t duration_ms = 60000);
    bool stopJamming();
    bool isJamming();
    
    struct Stats {
        uint32_t jam_duration_ms;
        uint32_t packets_sent;
        uint8_t channels_targeted;
    };
    Stats getStats();
}

#endif
