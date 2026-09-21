#pragma once
#include <Arduino.h>
#include <vector>

// Hardware diagnostics and health checks at boot
// Tests each peripheral and reports detected/not-detected status
// Ensures firmware gracefully handles missing or misconfigured devices
namespace SystemDiagnostics {

struct DiagnosticResult {
    String component;     // "RTC", "GPS", "TFT", "OLED", "CC1101", etc.
    bool detected;        // true if hardware responded
    String status;        // "OK" / "MISSING" / "ERROR: reason"
};

// Run full hardware diagnostics at boot
// Returns vector of all component statuses
std::vector<DiagnosticResult> runDiagnostics();

// Display diagnostic results on screen and log to serial
// Returns true if all critical components detected
bool showDiagnosticResults(const std::vector<DiagnosticResult> &results);

// Individual diagnostic tests (called by runDiagnostics)
namespace {
DiagnosticResult testRTC();
DiagnosticResult testGPS();
DiagnosticResult testDisplay();
DiagnosticResult testCC1101();
DiagnosticResult testNRF24();
DiagnosticResult testBattery();
DiagnosticResult testBuzzer();
DiagnosticResult testButtons();
DiagnosticResult testStorage();
DiagnosticResult testPSRAM();
}

} // namespace SystemDiagnostics
