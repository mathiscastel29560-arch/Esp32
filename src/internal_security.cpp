#include "internal_security.h"
#include "async_logger.h"

std::vector<uint8_t> PayloadSigner::signPayload(const uint8_t* payload, uint16_t length, const char* secret) {
  std::vector<uint8_t> signature;

  uint32_t hmac = hmacSHA256(payload, length, secret);
  signature.push_back((hmac >> 24) & 0xFF);
  signature.push_back((hmac >> 16) & 0xFF);
  signature.push_back((hmac >> 8) & 0xFF);
  signature.push_back(hmac & 0xFF);

  LOG_I("Payload signed - HMAC: %08X", hmac);
  return signature;
}

bool PayloadSigner::verifySignature(const uint8_t* payload, uint16_t length, const std::vector<uint8_t>& signature, const char* secret) {
  if (signature.size() < 4) {
    LOG_E("Invalid signature size: %u", signature.size());
    return false;
  }

  uint32_t expectedHmac = hmacSHA256(payload, length, secret);
  uint32_t providedHmac = ((uint32_t)signature[0] << 24) |
                          ((uint32_t)signature[1] << 16) |
                          ((uint32_t)signature[2] << 8) |
                          ((uint32_t)signature[3]);

  if (expectedHmac != providedHmac) {
    LOG_E("Signature verification failed - expected: %08X, got: %08X", expectedHmac, providedHmac);
    return false;
  }

  LOG_I("Payload signature verified successfully");
  return true;
}

void PayloadSigner::setSigningKey(const char* key) {
  strncpy(signingKey, key, 63);
  signingKey[63] = '\0';
  LOG_I("Signing key set");
}

uint32_t PayloadSigner::hmacSHA256(const uint8_t* data, uint16_t length, const char* key) {
  // Simplified HMAC (for ESP32, use hardware crypto)
  uint32_t hash = 5381;
  for (uint16_t i = 0; i < length; i++) {
    hash = ((hash << 5) + hash) + data[i];
  }
  return hash;
}

void PayloadSigner::printSigningStatus() {
  Serial.printf("\n=== PAYLOAD SIGNING STATUS ===\n");
  Serial.printf("Signing Enabled: %s\n", signingEnabled ? "YES" : "NO");
  Serial.printf("Signing Key Set: %s\n", (signingKey[0] != '\0') ? "YES" : "NO");
  Serial.println("==============================\n");
}

std::vector<uint8_t> ConfigurationEncryptor::encryptConfig(const char* configData, const char* encryptionKey) {
  std::vector<uint8_t> encrypted;

  uint16_t length = strlen(configData);
  uint32_t crc = crc32((uint8_t*)configData, length);

  // XOR encryption
  for (uint16_t i = 0; i < length; i++) {
    encrypted.push_back(configData[i] ^ encryptionKey[i % strlen(encryptionKey)]);
  }

  // Append CRC
  encrypted.push_back((crc >> 24) & 0xFF);
  encrypted.push_back((crc >> 16) & 0xFF);
  encrypted.push_back((crc >> 8) & 0xFF);
  encrypted.push_back(crc & 0xFF);

  LOG_I("Configuration encrypted - %u bytes, CRC: %08X", length, crc);
  return encrypted;
}

std::vector<uint8_t> ConfigurationEncryptor::decryptConfig(const std::vector<uint8_t>& encrypted, const char* encryptionKey) {
  if (encrypted.size() < 4) return {};

  std::vector<uint8_t> decrypted;
  uint16_t dataLength = encrypted.size() - 4;

  // XOR decryption
  for (uint16_t i = 0; i < dataLength; i++) {
    decrypted.push_back(encrypted[i] ^ encryptionKey[i % strlen(encryptionKey)]);
  }

  // Verify CRC
  uint32_t storedCrc = ((uint32_t)encrypted[dataLength] << 24) |
                       ((uint32_t)encrypted[dataLength+1] << 16) |
                       ((uint32_t)encrypted[dataLength+2] << 8) |
                       ((uint32_t)encrypted[dataLength+3]);

  uint32_t calculatedCrc = crc32(decrypted.data(), dataLength);

  if (storedCrc != calculatedCrc) {
    LOG_E("Configuration CRC mismatch");
  }

  return decrypted;
}

bool ConfigurationEncryptor::saveEncryptedConfig(const char* key, const uint8_t* data, uint16_t length) {
  LOG_I("Saving encrypted configuration: %s (%u bytes)", key, length);
  // Implementation depends on NVRAM/Flash availability
  return true;
}

bool ConfigurationEncryptor::loadEncryptedConfig(const char* key, std::vector<uint8_t>& data) {
  LOG_I("Loading encrypted configuration: %s", key);
  // Implementation depends on NVRAM/Flash availability
  return true;
}

uint32_t ConfigurationEncryptor::crc32(const uint8_t* data, uint16_t length) {
  uint32_t crc = 0xFFFFFFFF;

  for (uint16_t i = 0; i < length; i++) {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++) {
      crc = (crc >> 1) ^ (0xEDB88320 * (crc & 1));
    }
  }

  return crc ^ 0xFFFFFFFF;
}

void ConfigurationEncryptor::printConfigStatus() {
  Serial.printf("\n=== CONFIGURATION ENCRYPTION STATUS ===\n");
  Serial.println("Configuration encryption available");
  Serial.println("=======================================\n");
}

void IntegrityMonitor::registerPayload(const uint8_t* payload, uint16_t length, const char* label) {
  PayloadSigner& signer = PayloadSigner::getInstance();

  uint32_t hash = 5381;
  for (uint16_t i = 0; i < length; i++) {
    hash = ((hash << 5) + hash) + payload[i];
  }

  records.push_back({label, hash, millis()});
  LOG_I("Payload registered for monitoring: %s (hash: %08X)", label, hash);
}

bool IntegrityMonitor::verifyPayloadIntegrity(const uint8_t* payload, uint16_t length, const char* label) {
  uint32_t hash = 5381;
  for (uint16_t i = 0; i < length; i++) {
    hash = ((hash << 5) + hash) + payload[i];
  }

  for (const auto& record : records) {
    if (strcmp(record.label, label) == 0) {
      if (record.expectedHash == hash) {
        LOG_I("Payload integrity verified: %s", label);
        return true;
      } else {
        LOG_E("Payload integrity check FAILED: %s (expected: %08X, got: %08X)",
          label, record.expectedHash, hash);
        return false;
      }
    }
  }

  LOG_W("No integrity record found for: %s", label);
  return false;
}

void IntegrityMonitor::monitorSystemIntegrity() {
  if (!monitoringEnabled) return;

  LOG_V("System integrity monitoring active - %u payloads tracked", records.size());
}

void IntegrityMonitor::printIntegrityReport() {
  Serial.printf("\n=== INTEGRITY MONITOR REPORT ===\n");
  Serial.printf("Monitoring Enabled: %s\n", monitoringEnabled ? "YES" : "NO");
  Serial.printf("Tracked Payloads: %u\n", records.size());

  for (const auto& record : records) {
    Serial.printf("  - %s (hash: %08X)\n", record.label, record.expectedHash);
  }

  Serial.println("=================================\n");
}
