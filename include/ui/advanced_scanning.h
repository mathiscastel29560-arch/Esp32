#pragma once
#include <Arduino.h>
#include <vector>
#include "ui/theme.h"

// Advanced scanning UI: live progress visualization, animated status, and extended scan screens
// for network discovery tools (IoT, Zigbee, Z-Wave, RFID, etc).
namespace AdvancedScanning {

// ---- Live Scanning Progress Screen ----
// Display animated scanning progress with pulsing indicators and histogram buildup.
// Call this while actively scanning to show real-time progress.

struct ScanProgress {
    String title;              // "WiFi Scanning", "BLE Discovery", etc.
    uint16_t devicesFound;     // Devices/networks found so far
    uint32_t elapsedMs;        // Time spent scanning
    uint32_t totalDurationMs;  // Expected total scan duration
    bool isActive;             // Scanning in progress
    uint16_t signalStrength;   // Current signal bar value (0-100)
};

// Show live scanning progress screen with animated pulsing indicator
// Blocks until duration expires or button pressed. Returns true if button pressed early.
bool showScanProgress(const ScanProgress &progress);

// ---- IoT/Network Device Distribution Visualization ----
// Display discovered devices organized by type/protocol with distribution histogram

struct NetworkDeviceData {
    String title;                          // "IoT Devices", "Smart Locks", etc.
    uint16_t totalDevices;
    std::vector<uint16_t> devicesByType;   // Count per device type
    std::vector<String> typeLabels;        // "WiFi", "BLE", "Zigbee", "Z-Wave", etc.
    int32_t strongestSignal;               // Strongest RSSI observed
    String strongestDeviceName;
};

// Show network device distribution with histogram + gauge + type breakdown
void showNetworkDevices(const NetworkDeviceData &data);

// ---- Signal Quality Timeline Visualization ----
// Show RSSI values over time as a simple line/bar graph for a single device

struct SignalTimeline {
    String deviceName;
    std::vector<int32_t> rssiHistory;      // RSSI readings over time
    uint32_t samplingIntervalMs;           // Time between samples
};

// Show signal quality over time
void showSignalTimeline(const SignalTimeline &data);

// ---- Multiple Simultaneous Scans View ----
// Display progress of multiple concurrent scans (WiFi + BLE + RF) side-by-side

struct ConcurrentScanProgress {
    String wifiStatus;    // "25 networks", "Scanning...", etc.
    String bleStatus;     // "12 devices", "Scanning...", etc.
    String rfStatus;      // "8 peaks", "Analyzing...", etc.
    float wifiProgress;   // 0.0 to 1.0
    float bleProgress;
    float rfProgress;
};

// Show three concurrent scans with progress bars
void showConcurrentScans(const ConcurrentScanProgress &data);

// ---- Animated Loading/Scanning Indicator ----
// Lightweight animation for scan status (use while scanning, minimal CPU)

enum ScanState {
    SCAN_IDLE,
    SCAN_ACTIVE,
    SCAN_COMPLETE,
    SCAN_ERROR
};

// Draw animated scanning indicator at given position
void drawScanIndicator(int x, int y, ScanState state, uint32_t nowMs);

// ---- Helper: Build RSSI histogram with more granularity ----
// Creates 5-band histogram: very weak / weak / fair / good / excellent
std::vector<uint16_t> createDetailedRssiHistogram(const std::vector<int32_t> &rssiValues);

} // namespace AdvancedScanning
