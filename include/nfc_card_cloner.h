#ifndef NFC_CARD_CLONER_H
#define NFC_CARD_CLONER_H

#include <Arduino.h>
#include <vector>
#include <Wire.h>

class NFCCardCloner {
public:
    static NFCCardCloner& instance() {
        static NFCCardCloner cloner;
        return cloner;
    }

    struct NFCCard {
        char card_id[16];           // UID
        uint8_t card_type;          // 1=Type2, 2=Type4, 3=MIFARE
        std::vector<uint8_t> data;  // Full card data
        char card_name[32];         // User name
        uint32_t captured_time;
        uint8_t sector_count;
        uint16_t memory_size;       // Bytes
    };

    struct ClonerStats {
        uint32_t cards_read;
        uint32_t cards_stored;
        uint32_t cards_emulated;
        uint32_t read_attempts;
        uint32_t emulation_sessions;
        uint8_t success_rate;       // %
    };

    // Initialize PN532 via I2C
    bool begin();

    // Scan for nearby NFC cards
    bool scanNearby();

    // Read complete card data
    bool readCard(NFCCard& card);

    // Store card to persistent memory
    bool storeCard(const NFCCard& card);

    // Load stored card from memory
    bool loadCard(const char* card_id, NFCCard& card);

    // Start NFC emulation mode (peer-to-peer)
    bool startEmulation(const NFCCard& card);
    void stopEmulation();

    // Clone by storing then emulating
    bool cloneCard(const char* stored_id, const char* new_name);

    // List all stored cards
    std::vector<NFCCard> listStoredCards();

    // Delete stored card
    bool deleteCard(const char* card_id);

    ClonerStats getStats();
    String generateReport();
    void exportToJSON(const char* filepath);

private:
    NFCCardCloner() : initialized_(false), emulating_(false) {}

    bool initialized_;
    bool emulating_;
    std::vector<NFCCard> stored_cards_;
    NFCCard active_card_;
    ClonerStats stats_;

    bool initPN532();
    bool readUID(uint8_t* uid, uint8_t* uid_len);
    bool readSectors(NFCCard& card);
    bool validateNFCType(uint8_t type);
    uint8_t calculateChecksum(const std::vector<uint8_t>& data);
};

#endif
