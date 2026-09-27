#pragma once

#include <string>
#include <cstdint>
#include <functional>

// Delta OTA - Efficient incremental firmware updates
class DeltaOTA {
public:
  enum OTADeltaState {
    DELTA_IDLE = 0,
    DELTA_CHECKING = 1,
    DELTA_DOWNLOADING = 2,
    DELTA_PATCHING = 3,
    DELTA_VERIFYING = 4,
    DELTA_SUCCESS = 5,
    DELTA_ERROR = 6
  };

  using ProgressCallback = std::function<void(uint8_t, const std::string&)>;

  static DeltaOTA& getInstance() {
    static DeltaOTA instance;
    return instance;
  }

  // Check if delta update is available
  bool checkDeltaUpdate(const char* updateServerURL);

  // Download and apply delta patch
  bool downloadAndApplyDelta(const char* deltaURL);

  // Get current state
  OTADeltaState getState() const { return currentState; }

  // Get progress
  uint8_t getProgress() const { return progress; }

  // Set progress callback
  void setProgressCallback(ProgressCallback cb) { progressCallback = cb; }

  // Get delta size (smaller than full firmware)
  uint32_t getDeltaSize() const { return deltaSize; }

  // Get estimated time saved (bytes not downloaded)
  uint32_t getBytesSaved() const { return bytesSaved; }

  // Get last error
  std::string getLastError() const { return lastError; }

private:
  DeltaOTA() = default;

  OTADeltaState currentState = DELTA_IDLE;
  uint8_t progress = 0;
  uint32_t deltaSize = 0;
  uint32_t bytesSaved = 0;
  std::string lastError;
  ProgressCallback progressCallback = nullptr;

  // Helper functions
  bool calculateCurrentFirmwareHash();
  bool applyBinaryDiff(const uint8_t* deltaPatch, uint32_t patchSize);
  bool verifyPatchedFirmware();
};

#endif // DELTA_OTA_H
