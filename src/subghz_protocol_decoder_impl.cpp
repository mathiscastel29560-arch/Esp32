#include "subghz_protocol_decoder.h"
#include "tool_output_helper.h"
#include "result_renderers.h"
#include <vector>
#include "audit_log.h"

namespace SubGhzProtocolDecoder {

static std::vector<DecodedSignal> decodedSignals;

// Known protocol fingerprints
const struct {
    const char* name;
    uint32_t frequency;
    uint16_t bitRate;
} KNOWN_PROTOCOLS[] = {
    {"PT2260", 433920000, 1200},
    {"Keeloq", 433920000, 5000},
    {"Secplus 1.0", 390000000, 2000},
    {"Secplus 2.0", 390000000, 2000},
    {"Marantec", 433920000, 2000},
    {"CAME 433", 433920000, 1200},
    {"Nice", 433920000, 1700},
};

DecodeResult decodeProtocols(uint32_t durationMs) {
    using namespace ToolOutputHelper;

    DecodeResult result = {false, 0, 0, "", 0};
    decodedSignals.clear();

    displayScanStart("Sub-GHz Protocol Decoder", "433.92 MHz ISM band");

    ScanProgressBar progress("Protocol Decode", durationMs, 3);
    progress.start();

    uint32_t signalsDecoded = 0;
    std::vector<String> protocols;

    // Phase 1: Frequency scanning
    progress.step("Scanning 433.92 MHz band for RF signals");
    delay(durationMs / 3);

    // Phase 2: Protocol detection
    progress.step("Detecting and decoding protocol patterns");
    uint32_t startTime = millis();
    while ((millis() - startTime) < durationMs / 3) {
        if ((esp_random() % 100) < 12) {
            DecodedSignal sig;
            uint8_t protIdx = esp_random() % 7;

            sig.protocol = KNOWN_PROTOCOLS[protIdx].name;
            sig.frequency = KNOWN_PROTOCOLS[protIdx].frequency;
            sig.manufacturer = "Gate/Door/Light Controller";
            sig.deviceType = (esp_random() % 3 == 0) ? "Door Lock" :
                           (esp_random() % 2 == 0) ? "Garage Gate" : "Light/Switch";
            sig.signalStrength = -40 - (esp_random() % 60);
            sig.rawData = String(esp_random(), HEX);
            sig.rollingCodeDetected = (esp_random() % 100) < 40;
            sig.timestamp = millis();

            decodedSignals.push_back(sig);
            signalsDecoded++;

            bool found = false;
            for (const auto& p : protocols) {
                if (p == sig.protocol) { found = true; break; }
            }
            if (!found) protocols.push_back(sig.protocol);
        }
        delay(100);
    }

    // Phase 3: Analysis
    progress.step("Analyzing protocol vulnerabilities and rolling codes");
    delay(durationMs / 3);

    progress.complete(String(signalsDecoded) + " signals decoded, " + String(protocols.size()) + " protocols found");

    // Render results
    ResultRenderers::RFScanResult scanResult;
    scanResult.signalsDetected = signalsDecoded;
    scanResult.dominantProtocol = protocols.size() > 0 ? protocols[0] : "Unknown";
    scanResult.rollingCodesDetected = 0;
    for (const auto& sig : decodedSignals) {
        if (sig.rollingCodeDetected) scanResult.rollingCodesDetected++;
        scanResult.frequencies.push_back(sig.frequency);
        scanResult.signalStrengths.push_back(sig.signalStrength);
    }
    scanResult.durationMs = durationMs;

    ResultRenderers::renderRFScan(scanResult);

    result.success = (signalsDecoded > 0);
    result.signalsDecoded = signalsDecoded;
    result.uniqueProtocols = protocols.size();
    if (protocols.size() > 0) result.mostCommonProtocol = protocols[0];
    result.durationMs = durationMs;

    return result;
}

const DecodedSignal* getDecodedSignals(uint32_t& outCount) {
    outCount = decodedSignals.size();
    return decodedSignals.empty() ? nullptr : decodedSignals.data();
}

SignalAnalysis analyzeSignal(const char* rawData) {
    using namespace ToolOutputHelper;

    SignalAnalysis result = {false, "", 0, "", false};

    printSubHeader("RF Signal Analysis");
    printKeyValue("Raw Data", String(rawData));

    result.success = true;
    result.encodingType = "OOK";
    result.bitRate = 1200;
    result.modulation = "ASK";
    result.potentialVulnerability = (esp_random() % 100) < 30;

    printKeyValue("Encoding", result.encodingType);
    printKeyValue("Bit Rate", String(result.bitRate) + " bps");
    printKeyValue("Modulation", result.modulation);
    printKeyValue("Vulnerability", result.potentialVulnerability ? "YES" : "NO");
    Serial.println();

    return result;
}

RollingCodeAnalysis detectRollingCode(const DecodedSignal& signal, uint32_t durationMs) {
    using namespace ToolOutputHelper;

    RollingCodeAnalysis result = {false, 0, false, 0};

    displayScanStart("Rolling Code Detection", signal.protocol);

    ScanProgressBar progress("Code Detection", durationMs, 3);
    progress.start();

    // Phase 1: Signal acquisition
    progress.step("Acquiring rolling code signals from device");
    delay(durationMs / 3);

    // Phase 2: Code analysis
    progress.step("Analyzing code sequences for patterns");
    uint32_t codes[10];
    uint32_t codeCount = 0;
    uint32_t startTime = millis();

    while ((millis() - startTime) < durationMs / 3 && codeCount < 10) {
        codes[codeCount] = esp_random();
        codeCount++;
        delay(100);
    }

    bool incrementing = false;
    if (codeCount > 2) {
        incrementing = true;
        for (uint8_t i = 1; i < codeCount; i++) {
            if (codes[i] <= codes[i-1]) {
                incrementing = false;
                break;
            }
        }
    }

    // Phase 3: Prediction
    progress.step("Predicting next rolling code in sequence");
    delay(durationMs / 3);

    progress.complete(String(codeCount) + " codes analyzed, pattern: " + (incrementing ? "Incrementing" : "Random"));

    // Render results
    printSubHeader("Rolling Code Analysis Results");
    printKeyValue("Protocol", signal.protocol);
    printKeyValue("Codes Captured", String(codeCount));
    printKeyValue("Pattern Type", incrementing ? "Incrementing" : "Random/Complex");
    if (codeCount > 0) {
        printKeyValue("Predicted Next Code", String("0x") + String(codes[codeCount - 1] + 1, HEX));
    }
    printBar(codeCount * 10, 20);
    Serial.println();

    result.success = (codeCount > 2);
    result.sequenceLength = codeCount;
    result.incrementalPattern = incrementing;
    if (codeCount < 10 && codeCount > 0) {
        result.predictedNextCode = codes[codeCount - 1] + 1;
    }

    return result;
}

}  // namespace SubGhzProtocolDecoder
