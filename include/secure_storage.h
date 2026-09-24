#pragma once
#include <Arduino.h>
#include <map>
#include <vector>
#include <algorithm>

namespace SecureStorage {

struct EncryptionResult {
    bool success = false;
    uint32_t encryptedLength = 0;
    const char* error = "";
};

struct DecryptionResult {
    bool success = false;
    uint32_t decryptedLength = 0;
    const char* error = "";
};

struct CredentialsResult {
    bool success;
    String username;
    String password;
};

struct RateLimitStatus {
    bool allowed;
    uint32_t count;
};

// Crypto functions
bool calculateSHA256(const uint8_t* data, size_t length, uint8_t* hash);
EncryptionResult encryptData(const uint8_t* plaintext, size_t length, uint8_t* ciphertext);
DecryptionResult decryptData(const uint8_t* ciphertext, size_t length, uint8_t* plaintext);

// Credential management
bool saveCredentials(const char* key, const char* username, const char* password);
CredentialsResult loadCredentials(const char* key);

// Security logging
bool logSecureEvent(const char* event, const char* details);

// Rate limiting
RateLimitStatus checkRateLimit(const char* activity, uint32_t maxPerMinute);

// Integrity verification
bool verifyIntegrity(const char* filename);

} // namespace SecureStorage
