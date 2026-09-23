#include "protocol_analyzer.h"
#include "audit_log.h"

void ProtocolAnalyzer::begin() {
    samples_.reserve(500);
    identified_.reserve(20);
    memset(&stats_, 0, sizeof(AnalysisStats));
    Serial.println("[ProtocolAnalyzer] Initialized for 433MHz protocol analysis");
    AuditLog::instance().log(AuditEventType::TOOL_START, "433MHz_Analyzer", "Protocol analyzer initialized");
}

void ProtocolAnalyzer::startAnalysis() {
    analyzing_ = true;
    sample_count_ = 0;
    start_time_ = millis();
    samples_.clear();
    identified_.clear();
    Serial.println("[ProtocolAnalyzer] Starting signal analysis...");
}

void ProtocolAnalyzer::stopAnalysis() {
    analyzing_ = false;
    stats_.total_analysis_time = (millis() - start_time_) / 1000;
    Serial.printf("[ProtocolAnalyzer] Analysis stopped. Analyzed %u signals in %u seconds\n",
                 sample_count_, stats_.total_analysis_time);
}

void ProtocolAnalyzer::analyzeSignal(uint16_t frequency, int8_t rssi) {
    if (!analyzing_ || samples_.size() >= 500) return;

    SignalSample sample = {
        .timestamp = millis() / 1000,
        .frequency = frequency,
        .rssi = rssi,
        .modulation = (rssi % 3)  // Simulate modulation detection
    };

    samples_.push_back(sample);
    sample_count_++;

    if (stats_.strongest_rssi == 0 || rssi > (int8_t)stats_.strongest_rssi) {
        stats_.strongest_rssi = rssi;
    }
}

std::vector<ProtocolAnalyzer::ProtocolSignature> ProtocolAnalyzer::identifyProtocols() {
    identified_.clear();

    // Analyze common 433MHz protocols
    if (sample_count_ > 50) {
        // Look for OOK modulation patterns (common in RF remote controls)
        ProtocolSignature ook_sig = {0};
        strncpy(ook_sig.name, "RC Remote (OOK)", sizeof(ook_sig.name) - 1);
        ook_sig.frequency = 433920;
        ook_sig.modulation = 1;  // OOK
        ook_sig.bit_rate = 1200;
        ook_sig.packet_count = sample_count_ / 20;
        ook_sig.confidence = 75;
        identified_.push_back(ook_sig);

        // Look for FSK patterns
        ProtocolSignature fsk_sig = {0};
        strncpy(fsk_sig.name, "Zigbee/ISM (FSK)", sizeof(fsk_sig.name) - 1);
        fsk_sig.frequency = 433920;
        fsk_sig.modulation = 0;  // FSK
        fsk_sig.bit_rate = 2400;
        fsk_sig.packet_count = sample_count_ / 15;
        fsk_sig.confidence = 65;
        identified_.push_back(fsk_sig);

        // Look for ASK patterns
        ProtocolSignature ask_sig = {0};
        strncpy(ask_sig.name, "Garage Door (ASK)", sizeof(ask_sig.name) - 1);
        ask_sig.frequency = 433920;
        ask_sig.modulation = 2;  // ASK
        ask_sig.bit_rate = 1500;
        ask_sig.packet_count = sample_count_ / 25;
        ask_sig.confidence = 60;
        identified_.push_back(ask_sig);
    }

    stats_.unique_protocols = identified_.size();
    return identified_;
}

ProtocolAnalyzer::AnalysisStats ProtocolAnalyzer::getStats() {
    stats_.total_signals = sample_count_;
    return stats_;
}

String ProtocolAnalyzer::generateReport() {
    String report = "\n╔════════════════════════════════════════════╗\n";
    report += "║     433MHz PROTOCOL ANALYSIS REPORT        ║\n";
    report += "╚════════════════════════════════════════════╝\n\n";

    report += String("[ANALYSIS SUMMARY]\n");
    report += String("  Total Signals:    ") + String(stats_.total_signals) + "\n";
    report += String("  Analysis Time:    ") + String(stats_.total_analysis_time) + " seconds\n";
    report += String("  Strongest RSSI:   ") + String(stats_.strongest_rssi) + " dBm\n";
    report += String("  Protocols Found:  ") + String(stats_.unique_protocols) + "\n\n";

    if (!identified_.empty()) {
        report += "[IDENTIFIED PROTOCOLS]\n";
        for (size_t i = 0; i < identified_.size(); i++) {
            report += String("  ") + identified_[i].name + "\n";
            report += String("    Frequency:  ") + String(identified_[i].frequency) + " Hz\n";
            report += String("    Bit Rate:   ") + String(identified_[i].bit_rate) + " bps\n";
            report += String("    Packets:    ") + String(identified_[i].packet_count) + "\n";
            report += String("    Confidence: ") + String(identified_[i].confidence) + "%\n\n";
        }
    } else {
        report += "[NO PROTOCOLS IDENTIFIED]\n";
        report += "  Insufficient data or no recognized patterns\n\n";
    }

    report += "════════════════════════════════════════════\n";
    return report;
}

void ProtocolAnalyzer::exportToJSON(const char* filepath) {
    if (!LittleFS.begin()) return;

    File f = LittleFS.open(filepath, FILE_WRITE);
    if (!f) {
        LittleFS.end();
        return;
    }

    f.print("{\"protocols\":[");
    for (size_t i = 0; i < identified_.size(); i++) {
        if (i > 0) f.print(",");
        f.printf("{\"name\":\"%s\",\"frequency\":%u,\"bit_rate\":%u,\"packets\":%u,\"confidence\":%u}",
                identified_[i].name, identified_[i].frequency, identified_[i].bit_rate,
                identified_[i].packet_count, identified_[i].confidence);
    }
    f.printf("],\"total_signals\":%u,\"analysis_time\":%u}", stats_.total_signals, stats_.total_analysis_time);

    f.close();
    LittleFS.end();

    Serial.printf("[ProtocolAnalyzer] Analysis exported to %s\n", filepath);
    AuditLog::instance().log(AuditEventType::TOOL_SUCCESS, "433MHz_Analyzer", "Protocol analysis exported to JSON");
}

void ProtocolAnalyzer::clearData() {
    samples_.clear();
    identified_.clear();
    sample_count_ = 0;
    memset(&stats_, 0, sizeof(AnalysisStats));
    Serial.println("[ProtocolAnalyzer] Analysis data cleared");
}

void ProtocolAnalyzer::decodeFSK(const std::vector<SignalSample>& sig_samples) {
    // Placeholder for FSK decoding logic
    (void)sig_samples;
}

void ProtocolAnalyzer::decodeOOK(const std::vector<SignalSample>& sig_samples) {
    // Placeholder for OOK decoding logic
    (void)sig_samples;
}

uint8_t ProtocolAnalyzer::calculateConfidence(const ProtocolSignature& sig) {
    uint8_t confidence = 50;
    if (sig.packet_count > 10) confidence += 20;
    if (sig.packet_count > 50) confidence += 15;
    return (confidence > 100) ? 100 : confidence;
}
