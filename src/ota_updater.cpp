#include "ota_updater.h"
#include "audit_log.h"
#include "battery.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include <esp_ota_ops.h>

bool OtaUpdater::begin() {
    // Verify OTA partitions are available
    const esp_partition_t* running = esp_ota_get_running_partition();
    if (!running) {
        Serial.println("[OtaUpdater] No running partition found");
        return false;
    }

    snprintf(update_status_, sizeof(update_status_), "OTA ready");
    AuditLog::instance().log(AuditEventType::TOOL_START, "OtaUpdater", "Initialized");

    Serial.println("[OtaUpdater] Initialized and ready");
    return true;
}

bool OtaUpdater::checkForUpdates(const char* version_url) {
    if (!version_url) return false;

    if (WiFi.status() != WL_CONNECTED) {
        snprintf(update_status_, sizeof(update_status_), "WiFi not connected");
        return false;
    }

    HTTPClient http;
    http.begin(version_url);
    int httpCode = http.GET();

    if (httpCode != HTTP_CODE_OK) {
        snprintf(update_status_, sizeof(update_status_), "Failed to check updates (HTTP %d)", httpCode);
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    // Parse version from payload (simple format: "version=X.Y.Z")
    int pos = payload.indexOf("version=");
    if (pos >= 0) {
        payload.substring(pos + 8, pos + 13).toCharArray(latest_version_, sizeof(latest_version_));

        // Compare versions
        if (strcmp(latest_version_, current_version_) > 0) {
            snprintf(update_status_, sizeof(update_status_), "Update available: %s", latest_version_);
            Serial.printf("[OtaUpdater] Update available: %s -> %s\n", current_version_, latest_version_);
            return true;
        }
    }

    snprintf(update_status_, sizeof(update_status_), "Already up to date");
    return false;
}

bool OtaUpdater::updateFromUrl(const char* firmware_url, ProgressCallback progress) {
    if (!firmware_url) return false;

    // Check battery first
    if (Battery::isLow()) {
        snprintf(update_status_, sizeof(update_status_), "Battery too low for update");
        AuditLog::instance().log(AuditEventType::ERROR_OCCURRED, "OtaUpdater",
                                "Update aborted: low battery");
        return false;
    }

    if (WiFi.status() != WL_CONNECTED) {
        snprintf(update_status_, sizeof(update_status_), "WiFi not connected");
        return false;
    }

    is_updating_ = true;
    update_progress_ = 0;

    char details[96];
    snprintf(details, sizeof(details), "Starting OTA update from: %s", firmware_url);
    AuditLog::instance().log(AuditEventType::TOOL_START, "OtaUpdater", details);

    HTTPClient http;
    http.begin(firmware_url);
    http.addHeader("User-Agent", "ESP32-OTA");

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        snprintf(update_status_, sizeof(update_status_), "Failed to download (HTTP %d)", httpCode);
        http.end();
        is_updating_ = false;

        AuditLog::instance().log(AuditEventType::TOOL_FAILURE, "OtaUpdater",
                                "Download failed");
        return false;
    }

    int contentLength = http.getSize();
    if (contentLength <= 0) {
        snprintf(update_status_, sizeof(update_status_), "Invalid content length");
        http.end();
        is_updating_ = false;
        return false;
    }

    // Start OTA update
    if (!Update.begin(contentLength)) {
        snprintf(update_status_, sizeof(update_status_), "OTA begin failed");
        http.end();
        is_updating_ = false;

        AuditLog::instance().log(AuditEventType::ERROR_OCCURRED, "OtaUpdater",
                                "Failed to start OTA update");
        return false;
    }

    WiFiClient* stream = http.getStreamPtr();
    uint8_t buffer[256];
    int bytesWritten = 0;

    while (http.connected() && (bytesWritten < contentLength)) {
        size_t available = stream->available();
        if (available > 0) {
            int len = stream->readBytes(buffer, min((int)available, (int)sizeof(buffer)));
            if (len > 0) {
                if (Update.write(buffer, len) != len) {
                    snprintf(update_status_, sizeof(update_status_), "Write error at %d/%d",
                            bytesWritten, contentLength);
                    http.end();
                    Update.abort();
                    is_updating_ = false;
                    return false;
                }
                bytesWritten += len;
                update_progress_ = (bytesWritten * 100) / contentLength;

                if (progress) {
                    progress(bytesWritten, contentLength);
                }

                // Log every 25% progress
                if (update_progress_ % 25 == 0) {
                    Serial.printf("[OtaUpdater] Progress: %d%%\n", update_progress_);
                }
            }
        }
        delay(1);
    }

    http.end();

    if (Update.end()) {
        if (Update.isFinished()) {
            snprintf(update_status_, sizeof(update_status_), "Update successful, ready to reboot");
            snprintf(details, sizeof(details), "OTA update completed, new version: %s", latest_version_);
            AuditLog::instance().log(AuditEventType::TOOL_SUCCESS, "OtaUpdater", details);

            is_updating_ = false;
            update_progress_ = 100;
            Serial.println("[OtaUpdater] Update completed successfully!");
            return true;
        } else {
            snprintf(update_status_, sizeof(update_status_), "Update incomplete");
            is_updating_ = false;
            return false;
        }
    } else {
        snprintf(update_status_, sizeof(update_status_), "OTA error: %s", Update.errorString());
        AuditLog::instance().log(AuditEventType::ERROR_OCCURRED, "OtaUpdater",
                                Update.errorString());
        is_updating_ = false;
        return false;
    }
}

bool OtaUpdater::updateFromFile(const char* filepath) {
    if (!filepath) return false;

    if (!LittleFS.exists(filepath)) {
        snprintf(update_status_, sizeof(update_status_), "File not found: %s", filepath);
        return false;
    }

    File file = LittleFS.open(filepath, "r");
    if (!file) {
        snprintf(update_status_, sizeof(update_status_), "Cannot open file: %s", filepath);
        return false;
    }

    size_t fileSize = file.size();
    is_updating_ = true;
    update_progress_ = 0;

    if (!Update.begin(fileSize)) {
        snprintf(update_status_, sizeof(update_status_), "OTA begin failed");
        file.close();
        is_updating_ = false;
        return false;
    }

    uint8_t buffer[256];
    uint32_t bytesWritten = 0;

    while (file.available()) {
        size_t len = file.read(buffer, sizeof(buffer));
        if (len > 0) {
            if (Update.write(buffer, len) != len) {
                snprintf(update_status_, sizeof(update_status_), "Write error at %u/%u",
                        bytesWritten, (uint32_t)fileSize);
                file.close();
                Update.abort();
                is_updating_ = false;
                return false;
            }
            bytesWritten += len;
            update_progress_ = (bytesWritten * 100) / fileSize;
        }
    }

    file.close();

    if (Update.end() && Update.isFinished()) {
        snprintf(update_status_, sizeof(update_status_), "Update successful, ready to reboot");
        is_updating_ = false;
        update_progress_ = 100;
        return true;
    }

    snprintf(update_status_, sizeof(update_status_), "OTA error: %s", Update.errorString());
    is_updating_ = false;
    return false;
}

void OtaUpdater::rebootToApply() {
    if (!Update.isFinished()) {
        snprintf(update_status_, sizeof(update_status_), "No pending update");
        return;
    }

    Serial.println("[OtaUpdater] Rebooting to apply update...");
    AuditLog::instance().log(AuditEventType::TOOL_STOP, "OtaUpdater",
                            "Rebooting to apply update");

    delay(1000);
    ESP.restart();
}

bool OtaUpdater::rollbackToPrevious() {
    const esp_partition_t* prev = esp_ota_get_last_invalid_partition();
    if (!prev) {
        snprintf(update_status_, sizeof(update_status_), "No previous partition to rollback");
        return false;
    }

    esp_err_t err = esp_ota_set_boot_partition(prev);
    if (err != ESP_OK) {
        snprintf(update_status_, sizeof(update_status_), "Rollback failed: %d", err);
        return false;
    }

    Serial.println("[OtaUpdater] Rollback successful, rebooting...");
    AuditLog::instance().log(AuditEventType::CONFIG_CHANGED, "OtaUpdater",
                            "Rolled back to previous version");

    delay(1000);
    ESP.restart();
    return true;
}
