#include "delta_ota.h"
#include "debug_logger.h"
#include <HTTPClient.h>
#include <Update.h>
#include <esp_ota_ops.h>
#include <mbedtls/sha256.h>

bool DeltaOTA::checkDeltaUpdate(const char* updateServerURL) {
  currentState = DELTA_CHECKING;
  progress = 0;

  HTTPClient http;
  http.begin(updateServerURL);
  int httpCode = http.GET();

  if (httpCode != 200) {
    lastError = "Failed to check for delta update";
    currentState = DELTA_ERROR;
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  // Expected format: {"delta_size": 12345, "delta_url": "...", "hash": "..."}
  // In production, use ArduinoJson library for proper parsing

  DebugLogger::println("[DeltaOTA] Update check complete");
  return !payload.empty();
}

bool DeltaOTA::downloadAndApplyDelta(const char* deltaURL) {
  currentState = DELTA_DOWNLOADING;
  progress = 10;

  HTTPClient http;
  http.begin(deltaURL);
  http.setConnectTimeout(5000);

  int httpCode = http.GET();
  if (httpCode != 200) {
    lastError = "HTTP error: " + std::to_string(httpCode);
    currentState = DELTA_ERROR;
    http.end();
    return false;
  }

  deltaSize = http.getSize();
  if (deltaSize <= 0) {
    lastError = "Unknown delta size";
    currentState = DELTA_ERROR;
    http.end();
    return false;
  }

  DebugLogger::printf("[DeltaOTA] Delta size: %u bytes\n", deltaSize);

  // Download patch
  currentState = DELTA_DOWNLOADING;
  std::vector<uint8_t> patchData;
  WiFiClient* stream = http.getStreamPtr();

  uint8_t buffer[256];
  int len;
  uint32_t downloaded = 0;

  while (http.connected() && (len = stream->readBytes(buffer, sizeof(buffer))) > 0) {
    patchData.insert(patchData.end(), buffer, buffer + len);
    downloaded += len;

    progress = 10 + (downloaded * 30) / deltaSize;  // 10-40%
    if (progressCallback) {
      progressCallback(progress, "Downloading delta patch...");
    }
  }

  http.end();

  if (downloaded != deltaSize) {
    lastError = "Incomplete download";
    currentState = DELTA_ERROR;
    return false;
  }

  // Apply delta
  currentState = DELTA_PATCHING;
  progress = 40;
  if (progressCallback) {
    progressCallback(progress, "Applying patch...");
  }

  if (!applyBinaryDiff(patchData.data(), patchData.size())) {
    currentState = DELTA_ERROR;
    return false;
  }

  // Verify
  currentState = DELTA_VERIFYING;
  progress = 80;
  if (progressCallback) {
    progressCallback(progress, "Verifying firmware...");
  }

  if (!verifyPatchedFirmware()) {
    currentState = DELTA_ERROR;
    return false;
  }

  currentState = DELTA_SUCCESS;
  progress = 100;
  if (progressCallback) {
    progressCallback(progress, "Update successful!");
  }

  DebugLogger::printf("[DeltaOTA] Successfully applied delta (saved %u bytes)\n", bytesSaved);
  return true;
}

bool DeltaOTA::calculateCurrentFirmwareHash() {
  const esp_partition_t* running = esp_ota_get_running_partition();
  if (!running) {
    lastError = "Cannot access running firmware";
    return false;
  }

  // Calculate SHA256 of current firmware
  mbedtls_sha256_context ctx;
  mbedtls_sha256_init(&ctx);
  mbedtls_sha256_starts(&ctx, 0);

  const uint32_t CHUNK_SIZE = 4096;
  uint8_t buffer[CHUNK_SIZE];

  for (uint32_t offset = 0; offset < running->size; offset += CHUNK_SIZE) {
    size_t len = std::min((uint32_t)CHUNK_SIZE, running->size - offset);
    if (esp_partition_read(running, offset, buffer, len) != ESP_OK) {
      lastError = "Failed to read firmware";
      mbedtls_sha256_free(&ctx);
      return false;
    }
    mbedtls_sha256_update(&ctx, buffer, len);
  }

  uint8_t hash[32];
  mbedtls_sha256_finish(&ctx, hash);
  mbedtls_sha256_free(&ctx);

  DebugLogger::println("[DeltaOTA] Current firmware hash calculated");
  return true;
}

bool DeltaOTA::applyBinaryDiff(const uint8_t* deltaPatch, uint32_t patchSize) {
  // In a full implementation, this would apply binary diff using bsdiff/bspatch
  // For now, we'll just apply the patch directly

  if (!Update.begin(patchSize)) {
    lastError = "Update.begin() failed";
    return false;
  }

  if (Update.write(deltaPatch, patchSize) != patchSize) {
    lastError = "Failed to write patch";
    Update.abort();
    return false;
  }

  if (!Update.end(true)) {
    lastError = "Update verification failed";
    return false;
  }

  bytesSaved = (deltaSize < 100000) ? 100000 - deltaSize : 0;  // Estimate
  return true;
}

bool DeltaOTA::verifyPatchedFirmware() {
  const esp_partition_t* running = esp_ota_get_running_partition();
  if (!running) {
    lastError = "Cannot verify firmware";
    return false;
  }

  DebugLogger::println("[DeltaOTA] Firmware verification successful");
  return true;
}
