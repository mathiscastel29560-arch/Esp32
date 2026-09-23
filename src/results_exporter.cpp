#include "results_exporter.h"
#include "debug_logger.h"
#include <LittleFS.h>
#include <ctime>

namespace ResultsExporter {

const char *EXPORT_DIR = "/exports";
static uint32_t g_exportCount = 0;

void Exporter::begin() {
    // Create exports directory if needed
    File dir = LittleFS.open(EXPORT_DIR, "r");
    if (!dir) {
        LittleFS.mkdir(EXPORT_DIR);
    }

    DBG_INFO("ResultsExporter", "Export system initialized");
}

String Exporter::generateFilename(const String &prefix) {
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);

    char filename[64];
    snprintf(filename, sizeof(filename), "/exports/%s_%04d%02d%02d_%02d%02d%02d.json",
             prefix.c_str(),
             timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
             timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    return String(filename);
}

String Exporter::formatTimestamp(uint32_t timestamp) {
    time_t t = timestamp;
    struct tm *timeinfo = localtime(&t);

    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return String(buffer);
}

void Exporter::exportWiFiResults(const std::vector<WiFiResult> &results,
                                ExportFormat format,
                                const String &filename) {
    String exportFile = filename.length() > 0 ? filename : generateFilename("wifi_scan");

    if (format == FORMAT_JSON) {
        File file = LittleFS.open(exportFile, "w");
        if (!file) {
            DBG_ERROR("ResultsExporter", "Could not create export file");
            return;
        }

        file.println("{");
        file.print("  \"timestamp\": \"");
        file.print(formatTimestamp(time(nullptr)));
        file.println("\",");
        file.print("  \"type\": \"wifi_scan\",");
        file.print("  \"count\": ");
        file.print(results.size());
        file.println(",");
        file.println("  \"networks\": [");

        for (size_t i = 0; i < results.size(); i++) {
            const auto &r = results[i];
            file.print("    {");
            file.print("\"ssid\":\"");
            file.print(r.ssid);
            file.print("\",\"rssi\":");
            file.print(r.rssi);
            file.print(",\"channel\":");
            file.print(r.channel);
            file.print(",\"security\":\"");
            file.print(r.security);
            file.print("\",\"bssid\":\"");
            file.print(r.bssid);
            file.print("\"}");
            if (i < results.size() - 1) file.print(",");
            file.println();
        }

        file.println("  ]");
        file.println("}");
        file.close();

        DBG_INFO("ResultsExporter", "WiFi results exported to " + exportFile);
    }
    else if (format == FORMAT_CSV) {
        File file = LittleFS.open(exportFile.substring(0, exportFile.lastIndexOf('.')) + ".csv", "w");
        if (!file) {
            DBG_ERROR("ResultsExporter", "Could not create CSV file");
            return;
        }

        file.println("SSID,RSSI,Channel,Security,BSSID");
        for (const auto &r : results) {
            file.print(r.ssid);
            file.print(",");
            file.print(r.rssi);
            file.print(",");
            file.print(r.channel);
            file.print(",");
            file.print(r.security);
            file.print(",");
            file.println(r.bssid);
        }

        file.close();
        DBG_INFO("ResultsExporter", "WiFi results exported as CSV");
    }
}

void Exporter::exportBLEResults(const std::vector<BLEResult> &results,
                               ExportFormat format,
                               const String &filename) {
    String exportFile = filename.length() > 0 ? filename : generateFilename("ble_scan");

    if (format == FORMAT_JSON) {
        File file = LittleFS.open(exportFile, "w");
        if (!file) {
            DBG_ERROR("ResultsExporter", "Could not create export file");
            return;
        }

        file.println("{");
        file.print("  \"timestamp\": \"");
        file.print(formatTimestamp(time(nullptr)));
        file.println("\",");
        file.print("  \"type\": \"ble_scan\",");
        file.print("  \"count\": ");
        file.print(results.size());
        file.println(",");
        file.println("  \"devices\": [");

        for (size_t i = 0; i < results.size(); i++) {
            const auto &r = results[i];
            file.print("    {");
            file.print("\"name\":\"");
            file.print(r.name);
            file.print("\",\"address\":\"");
            file.print(r.address);
            file.print("\",\"rssi\":");
            file.print(r.rssi);
            file.print(",\"advdata\":\"");
            file.print(r.advData);
            file.print("\"}");
            if (i < results.size() - 1) file.print(",");
            file.println();
        }

        file.println("  ]");
        file.println("}");
        file.close();

        DBG_INFO("ResultsExporter", "BLE results exported to " + exportFile);
    }
    else if (format == FORMAT_CSV) {
        File file = LittleFS.open(exportFile.substring(0, exportFile.lastIndexOf('.')) + ".csv", "w");
        if (!file) return;

        file.println("Name,Address,RSSI,AdvertisementData");
        for (const auto &r : results) {
            file.print(r.name);
            file.print(",");
            file.print(r.address);
            file.print(",");
            file.print(r.rssi);
            file.print(",");
            file.println(r.advData);
        }

        file.close();
    }
}

void Exporter::exportRFResults(const std::vector<RFResult> &results,
                              ExportFormat format,
                              const String &filename) {
    String exportFile = filename.length() > 0 ? filename : generateFilename("rf_scan");

    if (format == FORMAT_JSON) {
        File file = LittleFS.open(exportFile, "w");
        if (!file) return;

        file.println("{");
        file.print("  \"timestamp\": \"");
        file.print(formatTimestamp(time(nullptr)));
        file.println("\",");
        file.print("  \"type\": \"rf_scan\",");
        file.print("  \"count\": ");
        file.print(results.size());
        file.println(",");
        file.println("  \"signals\": [");

        for (size_t i = 0; i < results.size(); i++) {
            const auto &r = results[i];
            file.print("    {");
            file.print("\"frequency\":");
            file.print(r.frequency, 3);
            file.print(",\"rssi\":");
            file.print(r.rssi);
            file.print(",\"timestamp\":");
            file.print(r.timestamp);
            file.print(",\"modulation\":\"");
            file.print(r.modulation);
            file.print("\"}");
            if (i < results.size() - 1) file.print(",");
            file.println();
        }

        file.println("  ]");
        file.println("}");
        file.close();

        DBG_INFO("ResultsExporter", "RF results exported to " + exportFile);
    }
    else if (format == FORMAT_CSV) {
        File file = LittleFS.open(exportFile.substring(0, exportFile.lastIndexOf('.')) + ".csv", "w");
        if (!file) return;

        file.println("Frequency(MHz),RSSI(dBm),Timestamp,Modulation");
        for (const auto &r : results) {
            file.print(r.frequency, 3);
            file.print(",");
            file.print(r.rssi);
            file.print(",");
            file.print(formatTimestamp(r.timestamp));
            file.print(",");
            file.println(r.modulation);
        }

        file.close();
    }
}

void Exporter::exportNFCResults(const std::vector<NFCResult> &results,
                               ExportFormat format,
                               const String &filename) {
    String exportFile = filename.length() > 0 ? filename : generateFilename("nfc_scan");

    if (format == FORMAT_JSON) {
        File file = LittleFS.open(exportFile, "w");
        if (!file) return;

        file.println("{");
        file.print("  \"timestamp\": \"");
        file.print(formatTimestamp(time(nullptr)));
        file.println("\",");
        file.print("  \"type\": \"nfc_scan\",");
        file.print("  \"count\": ");
        file.print(results.size());
        file.println(",");
        file.println("  \"cards\": [");

        for (size_t i = 0; i < results.size(); i++) {
            const auto &r = results[i];
            file.print("    {");
            file.print("\"uid\":\"");
            file.print(r.uid);
            file.print("\",\"type\":\"");
            file.print(r.type);
            file.print("\",\"data\":\"");
            file.print(r.data);
            file.print("\",\"timestamp\":");
            file.print(r.timestamp);
            file.print("}");
            if (i < results.size() - 1) file.print(",");
            file.println();
        }

        file.println("  ]");
        file.println("}");
        file.close();

        DBG_INFO("ResultsExporter", "NFC results exported to " + exportFile);
    }
    else if (format == FORMAT_CSV) {
        File file = LittleFS.open(exportFile.substring(0, exportFile.lastIndexOf('.')) + ".csv", "w");
        if (!file) return;

        file.println("UID,Type,Data,Timestamp");
        for (const auto &r : results) {
            file.print(r.uid);
            file.print(",");
            file.print(r.type);
            file.print(",");
            file.print(r.data);
            file.print(",");
            file.println(formatTimestamp(r.timestamp));
        }

        file.close();
    }
}

void Exporter::listExports() {
    File dir = LittleFS.open(EXPORT_DIR, "r");
    if (!dir) return;

    Serial.println();
    Serial.print(COLOR_CYAN);
    Serial.println("╔═══════════════════════════════════════╗");
    Serial.println("║ 📁 Exported Files");
    Serial.println("╠═══════════════════════════════════════╣");

    File file = dir.openNextFile();
    uint32_t totalSize = 0;
    int count = 0;

    while (file) {
        if (!file.isDirectory()) {
            String name = file.name();
            size_t fileSize = file.size();
            totalSize += fileSize;
            count++;

            Serial.print("║ ");
            Serial.print(name.substring(name.lastIndexOf('/') + 1));
            Serial.print(" (");
            Serial.print(fileSize);
            Serial.println(" bytes)");
        }
        file = dir.openNextFile();
    }

    Serial.print("║ Total: ");
    Serial.print(count);
    Serial.print(" files, ");
    Serial.print(totalSize);
    Serial.println(" bytes");
    Serial.println("╚═══════════════════════════════════════╝");
    Serial.print(COLOR_RESET);
}

uint32_t Exporter::getTotalExportSize() {
    File dir = LittleFS.open(EXPORT_DIR, "r");
    if (!dir) return 0;

    uint32_t totalSize = 0;
    File file = dir.openNextFile();

    while (file) {
        if (!file.isDirectory()) {
            totalSize += file.size();
        }
        file = dir.openNextFile();
    }

    return totalSize;
}

bool Exporter::deleteExport(const String &filename) {
    bool success = LittleFS.remove(filename);
    if (success) {
        DBG_INFO("ResultsExporter", "Deleted " + filename);
    } else {
        DBG_ERROR("ResultsExporter", "Failed to delete " + filename);
    }
    return success;
}

}  // namespace ResultsExporter
