#ifndef OTA_UPDATER_H
#define OTA_UPDATER_H

#include <Arduino.h>
#include <Update.h>

// Over-The-Air (OTA) firmware updates
// Download and install firmware updates without cable

class OtaUpdater {
public:
    static OtaUpdater& instance() {
        static OtaUpdater ota;
        return ota;
    }

    // Initialize OTA subsystem
    bool begin();

    // Check for updates from remote server
    // Returns true if update found
    bool checkForUpdates(const char* version_url);

    // Download and install firmware update from URL
    // Progress callback is called every ~1KB
    typedef void (*ProgressCallback)(uint32_t current, uint32_t total);
    bool updateFromUrl(const char* firmware_url, ProgressCallback progress = nullptr);

    // Update from local file (LittleFS or SPIFFS)
    bool updateFromFile(const char* filepath);

    // Get current firmware version
    const char* getCurrentVersion() const { return current_version_; }

    // Get latest available version
    const char* getLatestVersion() const { return latest_version_; }

    // Get update status
    bool isUpdating() const { return is_updating_; }
    uint8_t getUpdateProgress() const { return update_progress_; }
    const char* getUpdateStatus() const { return update_status_; }

    // Reboot to apply update
    void rebootToApply();

    // Rollback to previous version
    bool rollbackToPrevious();

private:
    OtaUpdater() : is_updating_(false), update_progress_(0) {
        snprintf(current_version_, sizeof(current_version_), "1.0.0");
        snprintf(latest_version_, sizeof(latest_version_), "1.0.0");
    }

    bool is_updating_;
    uint8_t update_progress_;
    char current_version_[32];
    char latest_version_[32];
    char update_status_[128];

    // Helper: download firmware and validate
    bool downloadAndValidateFirmware(const char* url, ProgressCallback progress);

    // Helper: write firmware to flash
    bool writeFirmwareToFlash(const uint8_t* data, uint32_t size);
};

#endif
