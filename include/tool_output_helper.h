#pragma once
#include <Arduino.h>
#include "result_renderers.h"

namespace ToolOutputHelper {

// ============================================================================
// UNIFIED TOOL OUTPUT WRAPPER - Use this for ALL tools!
// ============================================================================

class ScanProgressBar {
private:
    String toolName;
    uint32_t startTime;
    uint32_t totalDuration;
    uint8_t currentStep;
    uint8_t totalSteps;

public:
    ScanProgressBar(const String &name, uint32_t expectedDurationMs, uint8_t steps = 3);

    void start();
    void step(const String &message);
    void complete(const String &summary);

    uint8_t getPercentage() const;
    void drawProgressBar(uint8_t percent);
};

// ============================================================================
// TOOL OUTPUT TEMPLATES
// ============================================================================

// Generic scan output wrapper
void displayScanStart(const String &toolName, const String &targetInfo = "");
void displayScanProgress(uint8_t percent, const String &message);
void displayScanComplete(const String &summary, bool success);

// Attack output wrapper
void displayAttackStart(const String &attackName, uint32_t targetCount);
void displayAttackProgress(uint8_t percent, uint32_t currentTarget, const String &status);
void displayAttackComplete(uint32_t successCount, uint32_t totalTargets, bool anyCompromised);

// Device discovery output
void displayDeviceDiscovery(const String &deviceType, uint32_t count, int8_t signal);
void displayDeviceSummary(const String &type, uint32_t found, uint32_t vulnerable);

// ============================================================================
// BEAUTIFUL OUTPUT HELPERS
// ============================================================================

void printHeader(const String &title, const String &icon = "");
void printSubHeader(const String &title);
void printSuccess(const String &message);
void printWarning(const String &message);
void printError(const String &message);
void printInfo(const String &message);

void printBar(uint8_t percent, uint8_t width = 20);
void printKeyValue(const String &key, const String &value);
void printTable(const std::vector<String> &headers, const std::vector<std::vector<String>> &rows);

// ============================================================================
// EASY OUTPUT MACROS FOR TOOLS
// ============================================================================

// Quick output examples:
// TOOL_PRINT_SCAN_START("WiFi Scanner", "All channels");
// TOOL_PRINT_PROGRESS(45, "Scanning channel 6");
// TOOL_PRINT_SUCCESS("Network found: MySSID (-35 dBm)");
// TOOL_PRINT_SCAN_END("24 networks discovered", true);

}  // namespace ToolOutputHelper
