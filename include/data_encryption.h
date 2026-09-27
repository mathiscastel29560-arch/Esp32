#pragma once

#include <string>
#include <vector>
#include <cstdint>

// AES-256 encryption for sensitive data
class DataEncryption {
public:
  static DataEncryption& getInstance() {
    static DataEncryption instance;
    return instance;
  }

  // Initialize with master key (stored securely in NVS)
  bool begin();

  // Encrypt data with AES-256-GCM
  std::vector<uint8_t> encrypt(const std::string& plaintext);
  std::vector<uint8_t> encrypt(const std::vector<uint8_t>& plaintext);

  // Decrypt data with AES-256-GCM
  std::string decryptToString(const std::vector<uint8_t>& ciphertext);
  std::vector<uint8_t> decrypt(const std::vector<uint8_t>& ciphertext);

  // Generate random key (32 bytes for AES-256)
  static std::vector<uint8_t> generateKey();

  // Generate random IV (16 bytes for AES)
  static std::vector<uint8_t> generateIV();

  // Check if encryption is ready
  bool isReady() const { return keyInitialized; }

  // Encrypt audit logs before storing in NVS
  bool encryptAuditLog(const std::string& logEntry, std::string& encrypted);

  // Decrypt audit logs retrieved from NVS
  bool decryptAuditLog(const std::string& encrypted, std::string& logEntry);

private:
  DataEncryption() = default;

  bool keyInitialized = false;
  std::vector<uint8_t> masterKey;

  // Key derivation from stored seed
  bool deriveMasterKey();
};

#endif // DATA_ENCRYPTION_H
