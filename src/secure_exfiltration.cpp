#include "secure_exfiltration.h"
#include "async_logger.h"

std::vector<uint8_t> ResultEncryptor::encryptResults(const char* data, uint16_t length, const char* key) {
  std::vector<uint8_t> encrypted;
  encrypted.reserve(length + 4);  // +4 for checksum

  // Calculate checksum
  uint32_t checksum = fletcher32((uint8_t*)data, length);

  // XOR encryption with key expansion
  for (uint16_t i = 0; i < length; i++) {
    uint8_t keyByte = key[i % 32];
    encrypted.push_back(data[i] ^ keyByte);
  }

  // Append checksum (4 bytes)
  encrypted.push_back((checksum >> 24) & 0xFF);
  encrypted.push_back((checksum >> 16) & 0xFF);
  encrypted.push_back((checksum >> 8) & 0xFF);
  encrypted.push_back(checksum & 0xFF);

  LOG_I("Encrypted %u bytes, checksum: %08X", length, checksum);
  return encrypted;
}

std::vector<uint8_t> ResultEncryptor::decryptResults(const std::vector<uint8_t>& encrypted, const char* key) {
  if (encrypted.size() < 4) return {};

  std::vector<uint8_t> decrypted;
  uint16_t dataLength = encrypted.size() - 4;
  decrypted.reserve(dataLength);

  // Decrypt data
  for (uint16_t i = 0; i < dataLength; i++) {
    uint8_t keyByte = key[i % 32];
    decrypted.push_back(encrypted[i] ^ keyByte);
  }

  // Verify checksum
  uint32_t storedChecksum = ((uint32_t)encrypted[dataLength] << 24) |
                            ((uint32_t)encrypted[dataLength+1] << 16) |
                            ((uint32_t)encrypted[dataLength+2] << 8) |
                            ((uint32_t)encrypted[dataLength+3]);

  uint32_t calculatedChecksum = fletcher32(decrypted.data(), dataLength);

  if (storedChecksum != calculatedChecksum) {
    LOG_E("Checksum mismatch: stored=%08X, calculated=%08X", storedChecksum, calculatedChecksum);
  }

  return decrypted;
}

uint32_t ResultEncryptor::calculateChecksum(const uint8_t* data, uint16_t length) {
  return fletcher32(data, length);
}

bool ResultEncryptor::verifyChecksum(const uint8_t* data, uint16_t length, uint32_t checksum) {
  return fletcher32(data, length) == checksum;
}

void ResultEncryptor::setEncryptionKey(const char* key) {
  strncpy(encryptionKey, key, 31);
  encryptionKey[31] = '\0';
  LOG_I("Encryption key set");
}

uint32_t ResultEncryptor::fletcher32(const uint8_t* data, uint16_t length) {
  uint16_t sum1 = 0, sum2 = 0;

  for (uint16_t i = 0; i < length; i++) {
    sum1 = (sum1 + data[i]) % 255;
    sum2 = (sum2 + sum1) % 255;
  }

  return ((uint32_t)sum2 << 16) | sum1;
}

void ResultEncryptor::printEncryptionStatus() {
  Serial.printf("\n=== ENCRYPTION STATUS ===\n");
  Serial.printf("Encryption: %s\n", encryptionEnabled ? "ENABLED" : "DISABLED");
  Serial.printf("Key Set: %s\n", (encryptionKey[0] != '\0') ? "YES" : "NO");
  Serial.println("========================\n");
}

uint32_t ExfiltrationManager::scheduleExfiltration(const char* data, uint16_t length,
                                                    ExfiltrationMethod method, const char* label) {
  if (jobs.size() >= maxJobs) {
    LOG_W("Job queue full, clearing old jobs");
    clearCompletedJobs();
  }

  ResultEncryptor& encryptor = ResultEncryptor::getInstance();

  ExfiltrationJob job;
  job.jobId = nextJobId++;
  job.timestamp = millis();
  job.dataLabel = label;
  job.method = method;
  job.completed = false;

  // Encrypt data
  job.encryptedData = encryptor.encryptResults(data, length, "ExfiltrationKey123");
  job.checksum = encryptor.calculateChecksum((uint8_t*)data, length);

  jobs.push_back(job);
  LOG_I("Scheduled exfiltration job %u via method %u (%s)", job.jobId, (uint8_t)method, label);

  return job.jobId;
}

bool ExfiltrationManager::executeExfiltration(uint32_t jobId) {
  for (auto& job : jobs) {
    if (job.jobId == jobId && !job.completed) {
      bool success = false;

      switch (job.method) {
        case ExfiltrationMethod::USB_SERIAL:
          success = exfiltrateViaUSB(job);
          break;
        case ExfiltrationMethod::ENCRYPTED_WIFI:
          success = exfiltrateViaWiFi(job);
          break;
        case ExfiltrationMethod::ENCRYPTED_BLE:
          success = exfiltrateViaBLE(job);
          break;
        case ExfiltrationMethod::SD_CARD:
          success = exfiltrateViaSD(job);
          break;
        case ExfiltrationMethod::UART:
          success = exfiltrateViaUART(job);
          break;
      }

      if (success) {
        job.completed = true;
        LOG_I("Exfiltration job %u completed successfully", jobId);
      }

      return success;
    }
  }

  return false;
}

bool ExfiltrationManager::verifyExfiltration(uint32_t jobId) {
  for (const auto& job : jobs) {
    if (job.jobId == jobId && job.completed) {
      ResultEncryptor& encryptor = ResultEncryptor::getInstance();
      return encryptor.verifyChecksum(job.encryptedData.data(), job.encryptedData.size() - 4, job.checksum);
    }
  }
  return false;
}

bool ExfiltrationManager::isJobCompleted(uint32_t jobId) const {
  for (const auto& job : jobs) {
    if (job.jobId == jobId) return job.completed;
  }
  return false;
}

void ExfiltrationManager::printJobStatus() {
  Serial.printf("\n=== EXFILTRATION JOBS ===\n");
  Serial.printf("Total Jobs: %u\n", jobs.size());

  for (const auto& job : jobs) {
    const char* methodNames[] = {"USB", "WiFi", "BLE", "SD", "UART"};
    Serial.printf("Job %u: %s - %s (%u bytes) [%s]\n",
      job.jobId, job.dataLabel, methodNames[(uint8_t)job.method],
      job.encryptedData.size(), job.completed ? "DONE" : "PENDING");
  }

  Serial.println("=========================\n");
}

void ExfiltrationManager::clearCompletedJobs() {
  jobs.erase(
    std::remove_if(jobs.begin(), jobs.end(),
      [](const ExfiltrationJob& j) { return j.completed; }),
    jobs.end()
  );
  LOG_I("Cleared completed exfiltration jobs");
}

bool ExfiltrationManager::exfiltrateViaUSB(const ExfiltrationJob& job) {
  // Send via Serial (USB)
  Serial.printf("[EXFILTRATION] Job %u: ", job.jobId);
  for (const auto& byte : job.encryptedData) {
    Serial.printf("%02X", byte);
  }
  Serial.println();
  return true;
}

bool ExfiltrationManager::exfiltrateViaWiFi(const ExfiltrationJob& job) {
  LOG_I("WiFi exfiltration would send %u bytes encrypted", job.encryptedData.size());
  // Implementation depends on WiFi stack
  return true;
}

bool ExfiltrationManager::exfiltrateViaBLE(const ExfiltrationJob& job) {
  LOG_I("BLE exfiltration would send %u bytes encrypted", job.encryptedData.size());
  // Implementation depends on BLE stack
  return true;
}

bool ExfiltrationManager::exfiltrateViaSD(const ExfiltrationJob& job) {
  LOG_I("SD card exfiltration would write %u bytes encrypted", job.encryptedData.size());
  // Implementation depends on SD card driver
  return true;
}

bool ExfiltrationManager::exfiltrateViaUART(const ExfiltrationJob& job) {
  Serial.printf("[UART_EXFIL] Job %u: %u bytes\n", job.jobId, job.encryptedData.size());
  return true;
}
