#pragma once
#include <Arduino.h>
#include <vector>
#include "results_formatter.h"
#include "ui/advanced_widgets.h"

namespace ResultRenderers {

// ============= WiFi RENDERER =============
struct WiFiScanResult {
    uint32_t devicesFound;
    int32_t strongestRssi;
    String strongestSSID;
    std::vector<uint8_t> channelDistribution;  // Count per channel 1-14
    std::vector<int32_t> allRssiValues;
    uint32_t durationMs;
};

void renderWiFiScan(const WiFiScanResult &result);

// ============= BLE RENDERER =============
struct BLEScanResult {
    uint32_t devicesFound;
    int32_t strongestRssi;
    String strongestDevice;
    std::vector<int32_t> allRssiValues;
    uint32_t pairedDevices;
    uint32_t durationMs;
};

void renderBLEScan(const BLEScanResult &result);

// ============= RF/SubGHz RENDERER =============
struct RFScanResult {
    uint32_t signalsDetected;
    std::vector<uint32_t> frequencies;
    std::vector<int8_t> signalStrengths;
    String dominantProtocol;
    uint32_t rollingCodesDetected;
    uint32_t durationMs;
};

void renderRFScan(const RFScanResult &result);

// ============= NFC/RFID RENDERER =============
struct NFCScanResult {
    uint32_t cardsDetected;
    uint32_t relayedSuccessfully;
    std::vector<String> cardTypes;
    std::vector<int8_t> signalStrengths;
    uint32_t vulnerabilitiesFound;
    uint32_t durationMs;
};

void renderNFCScan(const NFCScanResult &result);

// ============= CELLULAR RENDERER =============
struct CellularScanResult {
    uint32_t devicesDetected;
    uint32_t imsiCaptured;
    std::vector<String> networkTypes;  // "2G", "3G", "4G", "5G"
    std::vector<int8_t> signalStrengths;
    uint32_t downgradeAttacks;
    uint32_t durationMs;
};

void renderCellularScan(const CellularScanResult &result);

// ============= IoT/MQTT RENDERER =============
struct IoTScanResult {
    uint32_t devicesFound;
    uint32_t brokersFound;
    uint32_t vulnerabilitiesDiscovered;
    std::vector<String> protocols;  // "MQTT", "CoAP", "Zigbee", etc
    std::vector<uint16_t> ports;
    uint32_t durationMs;
};

void renderIoTScan(const IoTScanResult &result);

// ============= ATTACK SUCCESS RENDERER =============
struct AttackSuccessResult {
    String attackName;
    bool success;
    uint32_t targetCount;
    uint32_t successCount;
    uint32_t failureCount;
    uint8_t successPercent;
    uint32_t durationMs;
};

void renderAttackSuccess(const AttackSuccessResult &result);

// ============= SIGNAL STRENGTH DISTRIBUTION RENDERER =============
struct SignalDistributionResult {
    String scanType;  // "WiFi", "BLE", "RF", etc
    std::vector<uint16_t> strengthBands;  // Weak/Fair/Good/Excellent
    uint32_t totalSignals;
    int32_t averageRssi;
    int32_t minRssi;
    int32_t maxRssi;
};

void renderSignalDistribution(const SignalDistributionResult &result);

// ============= EXPLOIT RESULTS RENDERER =============
struct ExploitResult {
    String exploitType;
    uint32_t attacksExecuted;
    uint32_t successfulExploits;
    uint32_t targetsCompromised;
    std::vector<String> vulnerabilitiesFound;
    uint8_t riskLevel;  // 0-100
    uint32_t durationMs;
};

void renderExploitResults(const ExploitResult &result);

// ============= HELPER: Progress with graphics =============
class ProgressRenderer {
public:
    static void start(const String &title, uint32_t totalSteps);
    static void update(uint32_t currentStep, const String &message, uint8_t percent);
    static void complete(const String &finalMessage);
};

}  // namespace ResultRenderers
