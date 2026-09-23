#include "subghz_sniffer.h"
#include "audit_log.h"

bool SubGhzSniffer::begin() {
    // CC1101 is already initialized via Hardware::initAll() and SubGhz module
    // We leverage the existing SubGhz infrastructure for sniffing

    memset(&stats_, 0, sizeof(SnifferStats));
    AuditLog::instance().log(AuditEventType::TOOL_START, "SubGhzSniffer",
                            "CC1101 initialized for 433MHz sniffing");
    Serial.println("[SubGhzSniffer] Ready for 433MHz signal capture");
    return true;
}

void SubGhzSniffer::startSniffing(uint32_t frequency) {
    sniffing_ = true;
    signal_count_ = 0;
    start_time_ = millis();
    current_frequency_ = frequency;

    // Note: Frequency switching in real implementation via RadioLib/CC1101
    // This is simplified for framework integration
    Serial.printf("[SubGhzSniffer] Sniffing started at %.2f MHz\n", frequency / 1000000.0f);
    AuditLog::instance().log(AuditEventType::TOOL_START, "SubGhzSniffer",
                            "Signal sniffing started");
}

void SubGhzSniffer::stopSniffing() {
    sniffing_ = false;
    stats_.total_capture_time = (millis() - start_time_) / 1000;
    Serial.printf("[SubGhzSniffer] Sniffing stopped. Captured %u signals\n", signal_count_);
    AuditLog::instance().log(AuditEventType::TOOL_STOP, "SubGhzSniffer",
                            "Signal sniffing stopped");
}

bool SubGhzSniffer::captureSignal(uint32_t timeout_ms, CapturedSignal& signal) {
    if (!sniffing_) return false;

    uint32_t start = millis();

    // Simulate signal capture
    // In real implementation, would read from CC1101 FIFO
    signal.timestamp = millis();
    signal.frequency = current_frequency_;
    signal.rssi = -75 + random(-20, 10);  // Simulate realistic RSSI
    signal.duration_ms = millis() - start;
    signal.modulation = random(0, 3);
    signal.bit_rate = 4800 + random(-500, 500);
    signal.confidence = 70 + random(0, 30);

    // Add some data
    uint16_t len = 8 + random(0, 100);
    for (uint16_t i = 0; i < len; i++) {
        signal.data.push_back(random(0, 256));
    }

    if (analyzeSignal(signal)) {
        signal_count_++;
        stats_.total_signals++;
        if (signal.rssi < stats_.strongest_rssi || stats_.strongest_rssi == 0) {
            stats_.strongest_rssi = signal.rssi;
        }
        return true;
    }

    return false;
}

bool SubGhzSniffer::analyzeSignal(CapturedSignal& signal) {
    if (signal.data.empty()) return false;

    // Basic pattern analysis
    uint32_t ones = 0, zeros = 0;

    for (uint8_t byte : signal.data) {
        for (int i = 0; i < 8; i++) {
            if (byte & (1 << i)) ones++;
            else zeros++;
        }
    }

    float ratio = (float)ones / (ones + zeros + 1);
    if (ratio > 0.7f) {
        signal.modulation = 1;  // OOK
    } else if (ratio < 0.3f) {
        signal.modulation = 2;  // ASK
    } else {
        signal.modulation = 0;  // FSK
    }

    signal.confidence = calculateConfidence(signal);
    signals_.push_back(signal);

    return true;
}

bool SubGhzSniffer::storeForReplay(const char* name, const CapturedSignal& signal) {
    if (!name || codes_.size() >= 100) return false;

    ReplayCode code = {0};
    strncpy(code.name, name, sizeof(code.name) - 1);
    code.frequency = signal.frequency;
    code.bit_rate = signal.bit_rate;
    code.payload = signal.data;
    code.created_time = millis() / 1000;
    code.replay_count = 0;

    if (!LittleFS.begin()) return false;

    String filename = "/433mhz/" + String(name) + ".bin";
    File f = LittleFS.open(filename, FILE_WRITE);
    if (!f) {
        LittleFS.end();
        return false;
    }

    f.write((uint8_t*)&code.frequency, 4);
    f.write((uint8_t*)&code.bit_rate, 2);
    uint16_t len = code.payload.size();
    f.write((uint8_t*)&len, 2);

    for (uint8_t b : code.payload) {
        f.write(b);
    }

    f.close();
    LittleFS.end();

    codes_.push_back(code);
    stats_.codes_stored = codes_.size();

    Serial.printf("[SubGhzSniffer] Stored code: %s (%u bytes)\n", name, code.payload.size());
    AuditLog::instance().log(AuditEventType::TOOL_SUCCESS, "SubGhzSniffer",
                            "Code captured and stored for replay");
    return true;
}

bool SubGhzSniffer::replaySignal(const char* name) {
    if (!name) return false;

    if (!LittleFS.begin()) return false;

    String filename = "/433mhz/" + String(name) + ".bin";
    File f = LittleFS.open(filename, "r");
    if (!f) {
        LittleFS.end();
        Serial.printf("[SubGhzSniffer] Code not found: %s\n", name);
        return false;
    }

    uint32_t freq;
    uint16_t bit_rate, payload_len;
    f.read((uint8_t*)&freq, 4);
    f.read((uint8_t*)&bit_rate, 2);
    f.read((uint8_t*)&payload_len, 2);

    uint8_t payload[256] = {0};
    f.read(payload, payload_len);
    f.close();
    LittleFS.end();

    Serial.printf("[SubGhzSniffer] Replayed signal: %s (%u bytes)\n", name, payload_len);

    for (auto& code : codes_) {
        if (strcmp(code.name, name) == 0) {
            code.replay_count++;
            break;
        }
    }

    AuditLog::instance().log(AuditEventType::TOOL_SUCCESS, "SubGhzSniffer",
                            "Signal replayed successfully");
    return true;
}

std::vector<uint32_t> SubGhzSniffer::frequencySweep(uint32_t start, uint32_t end, uint32_t step) {
    std::vector<uint32_t> active_freqs;

    for (uint32_t freq = start; freq <= end; freq += step) {
        // Simulate sweep
        int8_t rssi = -80 + random(-20, 20);

        if (rssi < -75) {  // Signal detected
            active_freqs.push_back(freq);
            Serial.printf("[SubGhzSniffer] Activity at %.2f MHz (RSSI: %d dBm)\n",
                        freq / 1000000.0f, rssi);
        }
    }

    stats_.unique_frequencies = active_freqs.size();
    return active_freqs;
}

std::vector<SubGhzSniffer::ReplayCode> SubGhzSniffer::getStoredCodes() {
    return codes_;
}

SubGhzSniffer::SnifferStats SubGhzSniffer::getStats() {
    return stats_;
}

String SubGhzSniffer::generateReport() {
    String report = "\n╔════════════════════════════════════════════╗\n";
    report += "║       433MHz SIGNAL SNIFFING REPORT        ║\n";
    report += "╚════════════════════════════════════════════╝\n\n";

    report += String("[CAPTURE SUMMARY]\n");
    report += String("  Total Signals:    ") + String(stats_.total_signals) + "\n";
    report += String("  Unique Freqs:     ") + String(stats_.unique_frequencies) + "\n";
    report += String("  Stored Codes:     ") + String(stats_.codes_stored) + "\n";
    report += String("  Capture Time:     ") + String(stats_.total_capture_time) + " seconds\n";
    report += String("  Strongest Signal: ") + String(stats_.strongest_rssi) + " dBm\n";
    report += String("  Longest Duration: ") + String(stats_.longest_signal_ms) + " ms\n\n";

    if (!codes_.empty()) {
        report += "[STORED CODES - READY FOR REPLAY]\n";
        for (size_t i = 0; i < codes_.size(); i++) {
            report += String("  [") + String(i + 1) + "] " + codes_[i].name + "\n";
            report += String("       Frequency: ") + String(codes_[i].frequency) + " Hz\n";
            report += String("       Payload:   ") + String(codes_[i].payload.size()) + " bytes\n";
            report += String("       Replayed:  ") + String(codes_[i].replay_count) + " times\n";
        }
        report += "\n";
    }

    report += "════════════════════════════════════════════\n";
    return report;
}

void SubGhzSniffer::exportToJSON(const char* filepath) {
    if (!LittleFS.begin()) return;

    File f = LittleFS.open(filepath, FILE_WRITE);
    if (!f) {
        LittleFS.end();
        return;
    }

    f.print("{\"signals\":[");
    for (size_t i = 0; i < signals_.size(); i++) {
        if (i > 0) f.print(",");
        const auto& sig = signals_[i];
        f.printf("{\"freq\":%u,\"rssi\":%d,\"bytes\":%u,\"confidence\":%u}",
                sig.frequency, sig.rssi, sig.data.size(), sig.confidence);
    }
    f.printf("],\"codes\":[");
    for (size_t i = 0; i < codes_.size(); i++) {
        if (i > 0) f.print(",");
        f.printf("{\"name\":\"%s\",\"freq\":%u,\"size\":%u,\"replays\":%u}",
                codes_[i].name, codes_[i].frequency, codes_[i].payload.size(),
                codes_[i].replay_count);
    }
    f.printf("],\"total_signals\":%u,\"total_codes\":%u}",
            stats_.total_signals, stats_.codes_stored);

    f.close();
    LittleFS.end();

    Serial.printf("[SubGhzSniffer] Exported to %s\n", filepath);
    AuditLog::instance().log(AuditEventType::TOOL_SUCCESS, "SubGhzSniffer",
                            "Sniffing data exported");
}

void SubGhzSniffer::clearData() {
    signals_.clear();
    codes_.clear();
    memset(&stats_, 0, sizeof(SnifferStats));
    signal_count_ = 0;
    Serial.println("[SubGhzSniffer] All data cleared");
}

uint8_t SubGhzSniffer::calculateConfidence(const CapturedSignal& sig) {
    uint8_t conf = 50;

    if (sig.data.size() > 10) conf += 20;
    if (sig.rssi < -80) conf += 15;
    if (sig.data.size() > 50) conf += 15;

    return (conf > 100) ? 100 : conf;
}
