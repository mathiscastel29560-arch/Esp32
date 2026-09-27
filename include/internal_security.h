#ifndef INTERNAL_SECURITY_H
#define INTERNAL_SECURITY_H

#include <Arduino.h>
#include <vector>

// ============= PAYLOAD SIGNING =============

class PayloadSigner {
public:
  static PayloadSigner& getInstance() {
    static PayloadSigner instance;
    return instance;
  }

  // Sign payload for integrity verification
  std::vector<uint8_t> signPayload(const uint8_t* payload, uint16_t length, const char* secret);
  bool verifySignature(const uint8_t* payload, uint16_t length, const std::vector<uint8_t>& signature, const char* secret);

  void setSigningKey(const char* key);
  bool isSigningEnabled() const { return signingEnabled; }

  void printSigningStatus();

private:
  PayloadSigner() : signingEnabled(true) {}

  char signingKey[64];
  bool signingEnabled;

  uint32_t hmacSHA256(const uint8_t* data, uint16_t length, const char* key);
};

// ============= CONFIGURATION ENCRYPTION =============

class ConfigurationEncryptor {
public:
  static ConfigurationEncryptor& getInstance() {
    static ConfigurationEncryptor instance;
    return instance;
  }

  // Encrypt sensitive configuration
  std::vector<uint8_t> encryptConfig(const char* configData, const char* encryptionKey);
  std::vector<uint8_t> decryptConfig(const std::vector<uint8_t>& encrypted, const char* encryptionKey);

  // NVRAM storage
  bool saveEncryptedConfig(const char* key, const uint8_t* data, uint16_t length);
  bool loadEncryptedConfig(const char* key, std::vector<uint8_t>& data);

  void printConfigStatus();

private:
  ConfigurationEncryptor() {}

  uint32_t crc32(const uint8_t* data, uint16_t length);
};

// ============= INTEGRITY MONITOR =============

class IntegrityMonitor {
public:
  static IntegrityMonitor& getInstance() {
    static IntegrityMonitor instance;
    return instance;
  }

  void registerPayload(const uint8_t* payload, uint16_t length, const char* label);
  bool verifyPayloadIntegrity(const uint8_t* payload, uint16_t length, const char* label);

  void monitorSystemIntegrity();
  void printIntegrityReport();

private:
  IntegrityMonitor() : monitoringEnabled(true) {}

  struct IntegrityRecord {
    const char* label;
    uint32_t expectedHash;
    uint32_t lastVerified;
  };

  std::vector<IntegrityRecord> records;
  bool monitoringEnabled;
};

#endif
