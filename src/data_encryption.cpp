#include "data_encryption.h"
#include "debug_logger.h"
#include <nvs_flash.h>
#include <esp_random.h>
#include <string.h>
#include <mbedtls/cipher.h>
#include <mbedtls/gcm.h>

bool DataEncryption::begin() {
  if (keyInitialized) return true;

  if (!deriveMasterKey()) {
    DebugLogger::println("[Encryption] Failed to derive master key");
    return false;
  }

  keyInitialized = true;
  DebugLogger::println("[Encryption] AES-256 encryption initialized");
  return true;
}

std::vector<uint8_t> DataEncryption::encrypt(const std::string& plaintext) {
  return encrypt(std::vector<uint8_t>(plaintext.begin(), plaintext.end()));
}

std::vector<uint8_t> DataEncryption::encrypt(const std::vector<uint8_t>& plaintext) {
  if (!keyInitialized) return std::vector<uint8_t>();

  std::vector<uint8_t> iv = generateIV();
  std::vector<uint8_t> ciphertext(plaintext.size() + 16);  // +16 for tag
  uint8_t tag[16] = {0};

  mbedtls_gcm_context ctx;
  mbedtls_gcm_init(&ctx);

  if (mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES,
                         masterKey.data(), masterKey.size() * 8) != 0) {
    DebugLogger::println("[Encryption] Failed to set encryption key");
    mbedtls_gcm_free(&ctx);
    return std::vector<uint8_t>();
  }

  if (mbedtls_gcm_crypt_and_tag(&ctx, MBEDTLS_GCM_ENCRYPT,
                                plaintext.size(),
                                iv.data(), iv.size(),
                                nullptr, 0,  // No additional data
                                plaintext.data(),
                                ciphertext.data(),
                                16, tag) != 0) {
    DebugLogger::println("[Encryption] Encryption failed");
    mbedtls_gcm_free(&ctx);
    return std::vector<uint8_t>();
  }

  mbedtls_gcm_free(&ctx);

  // Prepend IV + tag to ciphertext
  std::vector<uint8_t> result;
  result.insert(result.end(), iv.begin(), iv.end());
  result.insert(result.end(), tag, tag + 16);
  result.insert(result.end(), ciphertext.begin(),
                result.end() - 16); // Exclude padding tag from insert

  return result;
}

std::string DataEncryption::decryptToString(const std::vector<uint8_t>& ciphertext) {
  auto plaintext = decrypt(ciphertext);
  return std::string(plaintext.begin(), plaintext.end());
}

std::vector<uint8_t> DataEncryption::decrypt(const std::vector<uint8_t>& ciphertext) {
  if (!keyInitialized || ciphertext.size() < 32) {  // IV(16) + tag(16) + min data
    return std::vector<uint8_t>();
  }

  // Extract IV and tag
  std::vector<uint8_t> iv(ciphertext.begin(), ciphertext.begin() + 16);
  uint8_t tag[16];
  memcpy(tag, ciphertext.data() + 16, 16);

  // Encrypted data
  size_t encryptedSize = ciphertext.size() - 32;
  std::vector<uint8_t> plaintext(encryptedSize);

  mbedtls_gcm_context ctx;
  mbedtls_gcm_init(&ctx);

  if (mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES,
                         masterKey.data(), masterKey.size() * 8) != 0) {
    mbedtls_gcm_free(&ctx);
    return std::vector<uint8_t>();
  }

  if (mbedtls_gcm_crypt_and_tag(&ctx, MBEDTLS_GCM_DECRYPT,
                                encryptedSize,
                                iv.data(), iv.size(),
                                nullptr, 0,
                                ciphertext.data() + 32,
                                plaintext.data(),
                                16, tag) != 0) {
    DebugLogger::println("[Encryption] Decryption/tag verification failed");
    mbedtls_gcm_free(&ctx);
    return std::vector<uint8_t>();
  }

  mbedtls_gcm_free(&ctx);
  return plaintext;
}

std::vector<uint8_t> DataEncryption::generateKey() {
  std::vector<uint8_t> key(32);  // 256 bits

  for (size_t i = 0; i < key.size(); i++) {
    key[i] = esp_random() & 0xFF;
  }

  return key;
}

std::vector<uint8_t> DataEncryption::generateIV() {
  std::vector<uint8_t> iv(16);  // 128 bits

  for (size_t i = 0; i < iv.size(); i++) {
    iv[i] = esp_random() & 0xFF;
  }

  return iv;
}

bool DataEncryption::encryptAuditLog(const std::string& logEntry, std::string& encrypted) {
  auto ciphertext = encrypt(logEntry);
  if (ciphertext.empty()) return false;

  // Convert to hex string for NVS storage
  encrypted.clear();
  for (uint8_t byte : ciphertext) {
    char hex[3];
    snprintf(hex, sizeof(hex), "%02x", byte);
    encrypted += hex;
  }

  return true;
}

bool DataEncryption::decryptAuditLog(const std::string& encrypted, std::string& logEntry) {
  // Convert hex string to bytes
  std::vector<uint8_t> ciphertext;

  for (size_t i = 0; i < encrypted.length(); i += 2) {
    uint8_t byte = 0;
    sscanf(encrypted.substr(i, 2).c_str(), "%02x", (unsigned int*)&byte);
    ciphertext.push_back(byte);
  }

  logEntry = decryptToString(ciphertext);
  return !logEntry.empty();
}

bool DataEncryption::deriveMasterKey() {
  nvs_handle_t handle;
  esp_err_t err = nvs_open("encryption", NVS_READWRITE, &handle);
  if (err != ESP_OK) {
    DebugLogger::println("[Encryption] NVS open failed");
    return false;
  }

  char keyHex[65];
  size_t len = sizeof(keyHex);

  err = nvs_get_str(handle, "master_key", keyHex, &len);
  if (err == ESP_ERR_NVS_NOT_FOUND) {
    // Generate new key
    masterKey = generateKey();

    // Store as hex
    keyHex[0] = '\0';
    for (uint8_t byte : masterKey) {
      char hex[3];
      snprintf(hex, sizeof(hex), "%02x", byte);
      strcat(keyHex, hex);
    }

    nvs_set_str(handle, "master_key", keyHex);
    nvs_commit(handle);
    DebugLogger::println("[Encryption] Generated new master key");
  } else if (err == ESP_OK) {
    // Restore existing key from hex
    masterKey.clear();
    for (size_t i = 0; i < strlen(keyHex); i += 2) {
      uint8_t byte = 0;
      sscanf(keyHex + i, "%02x", (unsigned int*)&byte);
      masterKey.push_back(byte);
    }
    DebugLogger::println("[Encryption] Loaded existing master key");
  } else {
    DebugLogger::println("[Encryption] NVS read failed");
    nvs_close(handle);
    return false;
  }

  nvs_close(handle);
  return !masterKey.empty();
}
