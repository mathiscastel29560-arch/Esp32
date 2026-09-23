#pragma once
#include <Arduino.h>
#include <vector>

// Real-time results display for tools
namespace ResultsDisplay {

enum class ResultType {
    SUCCESS,
    WARNING,
    ERROR,
    INFO,
    SCAN_RESULT
};

struct DisplayResult {
    String title;
    String status;
    int progress;  // 0-100
    std::vector<String> lines;
    ResultType type;
};

// Display a tool result on TFT screen
void showResult(const String& toolName, const DisplayResult& result);

// Display scan progress
void updateProgress(int percent, const String& status);

// Clear results screen
void clearResults();

// Display table of results
void showTable(const String& title, const std::vector<String>& headers, const std::vector<std::vector<String>>& rows);

// Display signal strength bar
void showSignalBar(const String& label, int rssi);

// Display frequency/channel info
void showFrequencyInfo(uint16_t frequency, int8_t rssi, uint8_t channel);

}  // namespace ResultsDisplay
