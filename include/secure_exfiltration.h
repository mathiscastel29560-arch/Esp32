#ifndef SECURE_EXFILTRATION_H
#define SECURE_EXFILTRATION_H

#include <Arduino.h>
#include <vector>

// ============= RESULT ENCRYPTION =============

class ResultEncryptor {
public:
  static ResultEncryptor& getInstance() {
    static ResultEncryptor instance;
    return instance;
  }

  // Simple XOR + AES-128 simulation (for ESP32 capability)
  std::vector<uint8_t> encryptResults(const char* data, uint16_t length, const char* key);
  std::vector<uint8_t> decryptResults(const std::vector<uint8_t>& encrypted, const char* key);

  uint32_t calculateChecksum(const uint8_t* data, uint16_t length);
  bool verifyChecksum(const uint8_t* data, uint16_t length, uint32_t checksum);

  void setEncryptionKey(const char* key);
  void printEncryptionStatus();

private:
  ResultEncryptor() : encryptionEnabled(true) {}

  char encryptionKey[32];
  bool encryptionEnabled;

  uint32_t fletcher32(const uint8_t* data, uint16_t length);
};

// ============= SECURE EXFILTRATION MANAGER =============

enum class ExfiltrationMethod {
  USB_SERIAL = 0,
  ENCRYPTED_WIFI = 1,
  ENCRYPTED_BLE = 2,
  SD_CARD = 3,
  UART = 4
};

struct ExfiltrationJob {
  uint32_t jobId;
  uint32_t timestamp;
  const char* dataLabel;
  std::vector<uint8_t> encryptedData;
  uint32_t checksum;
  ExfiltrationMethod method;
  bool completed;
};

class ExfiltrationManager {
public:
  static ExfiltrationManager& getInstance() {
    static ExfiltrationManager instance;
    return instance;
  }

  uint32_t scheduleExfiltration(const char* data, uint16_t length,
                                ExfiltrationMethod method, const char* label);

  bool executeExfiltration(uint32_t jobId);
  bool verifyExfiltration(uint32_t jobId);

  uint16_t getPendingJobCount() const { return jobs.size(); }
  bool isJobCompleted(uint32_t jobId) const;

  void printJobStatus();
  void clearCompletedJobs();

private:
  ExfiltrationManager() : nextJobId(1000), maxJobs(20) {}

  std::vector<ExfiltrationJob> jobs;
  uint32_t nextJobId;
  uint16_t maxJobs;

  bool exfiltrateViaUSB(const ExfiltrationJob& job);
  bool exfiltrateViaWiFi(const ExfiltrationJob& job);
  bool exfiltrateViaBLE(const ExfiltrationJob& job);
  bool exfiltrateViaSD(const ExfiltrationJob& job);
  bool exfiltrateViaUART(const ExfiltrationJob& job);
};

#endif
