#pragma once

#include <stdint.h>
#include <string>
#include <functional>

// ============= FIRMWARE OTA UPDATE MANAGER =============

enum OTAState {
  OTA_IDLE,
  OTA_WAITING,
  OTA_DOWNLOADING,
  OTA_VERIFYING,
  OTA_FLASHING,
  OTA_SUCCESS,
  OTA_ERROR
};

struct OTAProgress {
  OTAState state;
  uint8_t percent;           // 0-100%
  std::string message;
  uint32_t bytesDownloaded;
  uint32_t totalBytes;
  uint32_t speed;            // bytes/sec
};

class FirmwareOTA {
public:
  static FirmwareOTA& getInstance() {
    static FirmwareOTA instance;
    return instance;
  }

  // Check for available updates
  bool checkForUpdates(const char* updateServerURL);

  // Start OTA update from URL
  bool startUpdate(const char* firmwareURL);

  // Get current OTA state
  OTAState getState() const { return currentState; }

  // Get progress information
  OTAProgress getProgress() const;

  // Handle OTA process (call in main loop)
  void handle();

  // Get version info
  struct VersionInfo {
    const char* currentVersion;
    const char* availableVersion;
    const char* releaseNotes;
    uint32_t releaseDate;
  };

  VersionInfo getVersionInfo() const;

  // Print update info
  void printUpdateInfo() const;

  // Callbacks
  using ProgressCallback = std::function<void(const OTAProgress&)>;
  void setProgressCallback(ProgressCallback cb) { progressCallback = cb; }

private:
  FirmwareOTA();

  OTAState currentState = OTA_IDLE;
  uint8_t progress = 0;
  std::string lastError;

  uint32_t bytesDownloaded = 0;
  uint32_t totalBytes = 0;
  uint32_t downloadStartTime = 0;

  uint32_t lastProgressUpdate = 0;
  ProgressCallback progressCallback = nullptr;

  // Helper functions
  void updateProgress(uint8_t percent, const std::string& message);
  bool validateFirmwareSignature(const uint8_t* data, uint32_t size);
  bool performRollback();
  std::string getLatestVersionURL() const;
};

#endif // FIRMWARE_OTA_H
