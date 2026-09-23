#ifndef RFID_CLONER_H
#define RFID_CLONER_H

#include <Arduino.h>

namespace RfidCloner {
    struct RfidCard {
        uint8_t uid[7];
        uint8_t uid_len;
        uint32_t timestamp;
        char card_type[32];
    };

    bool begin();
    bool scanCard(RfidCard& card, uint32_t timeout_ms = 5000);
    bool storeCard(const char* name, const RfidCard& card);
    bool emulateCard(const RfidCard& card);
    String listStoredCards();
    
    struct Stats {
        uint32_t cards_scanned;
        uint32_t cards_cloned;
        uint32_t emulations;
    };
    Stats getStats();
}

#endif
