#include "firmware_ota.h"
#include "debug_logger.h"
#include "audit_history.h"
#include <WiFi.h>
#include <Update.h>
#include <esp_ota_ops.h>
#include <HTTPClient.h>

FirmwareOTA::FirmwareOTA() {
}

bool FirmwareOTA::checkForUpdates(const char* updateServerURL) {
  if (WiFi.status() != WL_CONNECTED) {
    DebugLogger::println("[OTA] WiFi not connected");
    return false;
  }

  currentState = OTA_WAITING;
  updateProgress(0, "Checking for updates...");

  HTTPClient http;
  http.begin(updateServerURL);
  int httpCode = http.GET();

  bool updateAvailable = false;
  if (httpCode == 200) {
    String payload = http.getString();
    // Simple check: if response contains version number
    if (payload.length() > 0) {
      updateAvailable = true;
      DebugLogger::printf("[OTA] Update available: %s\n", payload.c_str());
    }
  }

  http.end();

  if (!updateAvailable) {
    currentState = OTA_IDLE;
    updateProgress(0, "No updates available");
  }

  return updateAvailable;
}

bool FirmwareOTA::startUpdate(const char* firmwareURL) {
  if (WiFi.status() != WL_CONNECTED) {
    DebugLogger::println("[OTA] WiFi not connected");
    lastError = "WiFi not connected";
    currentState = OTA_ERROR;
    return false;
  }

  currentState = OTA_DOWNLOADING;
  updateProgress(5, "Starting download...");

  HTTPClient http;
  http.begin(firmwareURL);
  http.setConnectTimeout(5000);
  http.setTimeout(10000);

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK && httpCode != HTTP_CODE_MOVED_PERMANENTLY) {
    DebugLogger::printf("[OTA] HTTP error: %d\n", httpCode);
    lastError = "HTTP error: " + String(httpCode);
    currentState = OTA_ERROR;
    http.end();
    return false;
  }

  totalBytes = http.getSize();
  if (totalBytes <= 0) {
    DebugLogger::println("[OTA] Unknown firmware size");
    lastError = "Unknown firmware size";
    currentState = OTA_ERROR;
    http.end();
    return false;
  }

  DebugLogger::printf("[OTA] Firmware size: %u bytes\n", totalBytes);

  // Start OTA update
  if (!Update.begin(totalBytes)) {
    DebugLogger::printf("[OTA] Update.begin() failed: %s\n", Update.errorString());
    lastError = Update.errorString();
    currentState = OTA_ERROR;
    http.end();
    return false;
  }

  bytesDownloaded = 0;
  downloadStartTime = millis();

  // Get stream and write to Update
  WiFiClient* stream = http.getStreamPtr();
  uint8_t buffer[256];
  int len;

  while (http.connected() && (len = stream->readBytes(buffer, sizeof(buffer))) > 0) {
    if (Update.write(buffer, len) != len) {
      DebugLogger::printf("[OTA] Write failed: %s\n", Update.errorString());
      lastError = Update.errorString();
      currentState = OTA_ERROR;
      Update.abort();
      http.end();
      return false;
    }

    bytesDownloaded += len;
    uint8_t percent = (bytesDownloaded * 100) / totalBytes;

    if (millis() - lastProgressUpdate > 1000) {
      uint32_t speed = (bytesDownloaded * 1000) / (millis() - downloadStartTime);
      DebugLogger::printf("[OTA] Progress: %u%% (%u/%u bytes) - %u KB/s\n",
        percent, bytesDownloaded, totalBytes, speed / 1024);
      updateProgress(5 + (percent * 45 / 100), "Downloading firmware...");
      lastProgressUpdate = millis();
    }
  }

  http.end();

  if (bytesDownloaded != totalBytes) {
    DebugLogger::printf("[OTA] Download incomplete: %u/%u bytes\n",
      bytesDownloaded, totalBytes);
    lastError = "Incomplete download";
    currentState = OTA_ERROR;
    Update.abort();
    return false;
  }

  // Finalize update
  currentState = OTA_VERIFYING;
  updateProgress(55, "Verifying firmware...");

  if (!Update.end(true)) {
    DebugLogger::printf("[OTA] Verification failed: %s\n", Update.errorString());
    lastError = Update.errorString();
    currentState = OTA_ERROR;
    return false;
  }

  currentState = OTA_FLASHING;
  updateProgress(95, "Flashing firmware...");

  // At this point, the device will restart automatically with new firmware
  currentState = OTA_SUCCESS;
  updateProgress(100, "Update successful! Restarting...");

  DebugLogger::println("[OTA] Firmware update successful!");
  DebugLogger::println("[OTA] Device will restart in 3 seconds...");

  // Restart after delay
  delay(3000);
  ESP.restart();

  return true;
}

OTAProgress FirmwareOTA::getProgress() const {
  OTAProgress prog;
  prog.state = currentState;
  prog.percent = progress;
  prog.message = "";
  prog.bytesDownloaded = bytesDownloaded;
  prog.totalBytes = totalBytes;

  if (downloadStartTime > 0) {
    uint32_t elapsed = millis() - downloadStartTime;
    if (elapsed > 0) {
      prog.speed = (bytesDownloaded * 1000) / elapsed;
    }
  }

  switch (currentState) {
    case OTA_IDLE:
      prog.message = "No update in progress";
      break;
    case OTA_WAITING:
      prog.message = "Waiting to start update";
      break;
    case OTA_DOWNLOADING:
      prog.message = "Downloading firmware...";
      break;
    case OTA_VERIFYING:
      prog.message = "Verifying firmware...";
      break;
    case OTA_FLASHING:
      prog.message = "Flashing firmware...";
      break;
    case OTA_SUCCESS:
      prog.message = "Update successful!";
      break;
    case OTA_ERROR:
      prog.message = "Error: " + lastError;
      break;
  }

  return prog;
}

void FirmwareOTA::handle() {
  if (currentState == OTA_DOWNLOADING) {
    // Progress already handled in startUpdate
  }
}

FirmwareOTA::VersionInfo FirmwareOTA::getVersionInfo() const {
  static VersionInfo info = {
    "2.0.0",           // Current version
    "2.0.1",           // Available version (would come from server)
    "Bug fixes and improvements",
    1704067600         // 2024-01-01
  };

  return info;
}

void FirmwareOTA::updateProgress(uint8_t percent, const std::string& message) {
  progress = percent;

  if (progressCallback) {
    OTAProgress prog = getProgress();
    prog.percent = percent;
    prog.message = message;
    progressCallback(prog);
  }
}

void FirmwareOTA::printUpdateInfo() const {
  auto version = getVersionInfo();
  auto prog = getProgress();

  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║      FIRMWARE UPDATE INFORMATION       ║");
  Serial.println("╚════════════════════════════════════════╝");

  Serial.printf("📦 Current Version:    %s\n", version.currentVersion);
  Serial.printf("🆕 Latest Version:     %s\n", version.availableVersion);
  Serial.printf("📝 Release Notes:      %s\n", version.releaseNotes);
  Serial.printf("📅 Release Date:       %u\n", version.releaseDate);

  Serial.printf("\n📊 Update Status:      ");
  switch (prog.state) {
    case OTA_IDLE: Serial.println("Idle"); break;
    case OTA_WAITING: Serial.println("Waiting"); break;
    case OTA_DOWNLOADING: Serial.println("Downloading"); break;
    case OTA_VERIFYING: Serial.println("Verifying"); break;
    case OTA_FLASHING: Serial.println("Flashing"); break;
    case OTA_SUCCESS: Serial.println("✓ Success"); break;
    case OTA_ERROR: Serial.println("✗ Error"); break;
  }

  if (prog.state == OTA_DOWNLOADING || prog.state == OTA_VERIFYING || prog.state == OTA_FLASHING) {
    Serial.printf("Progress:              %u%%\n", prog.percent);
    Serial.printf("Downloaded:            %u / %u bytes\n", prog.bytesDownloaded, prog.totalBytes);

    if (prog.speed > 0) {
      Serial.printf("Speed:                 %u KB/s\n", prog.speed / 1024);
    }
  }

  Serial.println("");
}
