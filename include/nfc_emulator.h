#ifndef NFC_EMULATOR_H
#define NFC_EMULATOR_H

#include <Arduino.h>
#include <vector>

class NFCEmulator {
public:
    static NFCEmulator& instance() {
        static NFCEmulator ne;
        return ne;
    }

    struct NFCTag {
        char tag_id[16];
        uint8_t tag_type;  // 1=Type2, 2=Type4, 3=MIFARE
        char ndef_text[256];
        uint8_t ndef_length;
        uint32_t created_time;
        uint32_t access_count;
    };

    struct EmulationStats {
        uint32_t total_scans;
        uint32_t emulated_tags;
        uint32_t read_attempts;
        uint32_t write_attempts;
        uint32_t uptime_seconds;
        uint8_t success_rate;
    };

    void begin();
    void loadTag(const char* tag_file);
    void createTag(const char* tag_id, uint8_t type, const char* ndef_data);
    void startEmulation();
    void stopEmulation();
    void deleteTag(const char* tag_id);

    std::vector<NFCTag> listTags();
    EmulationStats getStats();
    String generateReport();
    void exportTagsToJSON(const char* filepath);
    void importTagsFromJSON(const char* filepath);

private:
    NFCEmulator() : emulating_(false), scan_count_(0) {}

    bool emulating_;
    std::vector<NFCTag> stored_tags_;
    EmulationStats stats_;
    uint32_t scan_count_;
    uint32_t start_time_;

    bool validateNDEF(const char* data);
    uint8_t calculateChecksum(const NFCTag& tag);
};

#endif
