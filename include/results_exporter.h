#pragma once
#include <Arduino.h>
#include <vector>

namespace ResultsExporter {

// WiFi scan result
struct WiFiResult {
    String ssid;
    int32_t rssi;
    uint8_t channel;
    String security;
    String bssid;
};

// BLE device result
struct BLEResult {
    String name;
    String address;
    int32_t rssi;
    String advData;
};

// RF signal result
struct RFResult {
    float frequency;
    int32_t rssi;
    uint32_t timestamp;
    String modulation;
};

// NFC card result
struct NFCResult {
    String uid;
    String type;
    String data;
    uint32_t timestamp;
};

// Export formats
enum ExportFormat {
    FORMAT_CSV,
    FORMAT_JSON,
    FORMAT_HTML,
    FORMAT_TXT
};

class Exporter {
public:
    // Initialize exporter
    static void begin();

    // WiFi exports
    static void exportWiFiResults(const std::vector<WiFiResult> &results,
                                 ExportFormat format = FORMAT_JSON,
                                 const String &filename = "");

    // BLE exports
    static void exportBLEResults(const std::vector<BLEResult> &results,
                                ExportFormat format = FORMAT_JSON,
                                const String &filename = "");

    // RF exports
    static void exportRFResults(const std::vector<RFResult> &results,
                               ExportFormat format = FORMAT_JSON,
                               const String &filename = "");

    // NFC exports
    static void exportNFCResults(const std::vector<NFCResult> &results,
                                ExportFormat format = FORMAT_JSON,
                                const String &filename = "");

    // Utility functions
    static String generateFilename(const String &prefix);
    static String formatTimestamp(uint32_t timestamp);
    static void listExports();
    static uint32_t getTotalExportSize();
    static bool deleteExport(const String &filename);
};

}  // namespace ResultsExporter
