#ifndef SUBGHZ_SNIFFER_H
#define SUBGHZ_SNIFFER_H

#include <Arduino.h>
#include <RadioLib.h>
#include <vector>

class SubGhzSniffer {
public:
    static SubGhzSniffer& instance() {
        static SubGhzSniffer sniffer;
        return sniffer;
    }

    struct CapturedSignal {
        uint32_t timestamp;
        uint32_t frequency;      // Hz
        int8_t rssi;             // dBm
        uint32_t duration_ms;    // Signal duration
        float bandwidth;         // kHz
        uint8_t modulation;      // 0=FSK, 1=OOK, 2=ASK
        uint16_t bit_rate;       // bps
        std::vector<uint8_t> data;  // Raw captured bytes
        uint8_t confidence;      // 0-100%
    };

    struct ReplayCode {
        char name[32];
        uint32_t frequency;
        uint16_t bit_rate;
        std::vector<uint8_t> payload;
        uint32_t created_time;
        uint16_t replay_count;
    };

    struct SnifferStats {
        uint32_t total_signals;
        uint32_t unique_frequencies;
        uint32_t total_capture_time;
        int8_t strongest_rssi;
        uint32_t longest_signal_ms;
        uint32_t codes_stored;
    };

    // Initialize CC1101 for sniffing
    bool begin();

    // Start capturing signals on a frequency
    void startSniffing(uint32_t frequency = 433920000);  // 433.92 MHz default
    void stopSniffing();

    // Capture single signal with timeout
    bool captureSignal(uint32_t timeout_ms, CapturedSignal& signal);

    // Analyze captured signal pattern
    bool analyzeSignal(CapturedSignal& signal);

    // Store signal for replay
    bool storeForReplay(const char* name, const CapturedSignal& signal);

    // Replay stored signal
    bool replaySignal(const char* name);

    // Frequency sweep to find active signals
    std::vector<uint32_t> frequencySweep(uint32_t start, uint32_t end, uint32_t step);

    // Get stored codes
    std::vector<ReplayCode> getStoredCodes();
    SnifferStats getStats();
    String generateReport();
    void exportToJSON(const char* filepath);
    void clearData();

private:
    SubGhzSniffer() : sniffing_(false), signal_count_(0) {}

    bool sniffing_;
    std::vector<CapturedSignal> signals_;
    std::vector<ReplayCode> codes_;
    SnifferStats stats_;
    uint32_t signal_count_;
    uint32_t start_time_;
    uint32_t current_frequency_;

    void processRawData(uint8_t* data, uint16_t len, int8_t rssi, CapturedSignal& sig);
    uint8_t calculateConfidence(const CapturedSignal& sig);
};

#endif
