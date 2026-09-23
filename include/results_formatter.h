#pragma once
#include <Arduino.h>
#include <vector>

namespace ResultsFormatter {

// Result types
enum ResultType {
    RESULT_SUCCESS,
    RESULT_WARNING,
    RESULT_ERROR,
    RESULT_INFO,
    RESULT_SCAN
};

// Stat entry
struct StatEntry {
    String label;
    String value;
    String unit = "";
};

// Scan result entry
struct ScanEntry {
    String name;
    String signal;    // RSSI or signal strength
    String details;   // Additional info
};

// Display a formatted result
void displayResult(const String &title, const String &description,
                   ResultType type, uint8_t successPercent = 0);

// Display statistics in a nice format
void displayStats(const String &title, const std::vector<StatEntry> &stats);

// Display scan results as a table
void displayScanResults(const String &title, const std::vector<ScanEntry> &results);

// Display a data table
void displayTable(const String &title, const std::vector<String> &headers,
                  const std::vector<std::vector<String>> &rows);

// Display progress for long operations
void startProgress(const String &task, uint8_t steps);
void updateProgress(uint8_t step, const String &description = "");
void endProgress();

// Display success/error summary
void displaySummary(const String &title, uint32_t total, uint32_t success,
                    uint32_t failed, uint32_t skipped = 0);

// Draw signal strength visualization
void displaySignalMap(const String &channel, int rssi);

}  // namespace ResultsFormatter
