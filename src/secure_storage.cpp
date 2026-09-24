#include "secure_storage.h"
#include <LittleFS.h>
#include <mbedtls/aes.h>
#include <mbedtls/sha256.h>
#include <string.h>

namespace SecureStorage {

// Encryption key (in production, should be derived from hardware unique ID)
static uint8_t encryptionKey[32] = {
    0x48, 0x6F, 0x6D, 0x65, 0x53, 0x65, 0x63, 0x75,
    0x72, 0x69, 0x74, 0x79, 0x46, 0x69, 0x72, 0x73,
    0x74, 0x4C, 0x69, 0x6E, 0x65, 0x44, 0x65, 0x66,
    0x65, 0x6E, 0x73, 0x65, 0x53, 0x79, 0x73, 0x74
};

// IV (should be random per encryption in production)
static uint8_t initVector[16] = {
    0x49, 0x6E, 0x69, 0x74, 0x56, 0x65, 0x63, 0x74,
    0x6F, 0x72, 0x56, 0x31, 0x2E, 0x30, 0x00, 0x00
};

// Calculate SHA256 hash
bool calculateSHA256(const uint8_t* data, size_t length, uint8_t* hash) {
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);

    mbedtls_sha256_starts(&ctx, false);  // false = SHA256, true = SHA224
    mbedtls_sha256_update(&ctx, data, length);
    mbedtls_sha256_finish(&ctx, hash);
    mbedtls_sha256_free(&ctx);

    return true;
}

// Encrypt data using AES-256-CBC
EncryptionResult encryptData(const uint8_t* plaintext, size_t length, uint8_t* ciphertext) {
    EncryptionResult result;

    if (length == 0 || length % 16 != 0) {
        result.error = "Data length must be multiple of 16";
        return result;
    }

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);

    int ret = mbedtls_aes_setkey_enc(&aes, encryptionKey, 256);
    if (ret != 0) {
        result.error = "Failed to set AES key";
        mbedtls_aes_free(&aes);
        return result;
    }

    uint8_t iv_copy[16];
    memcpy(iv_copy, initVector, 16);

    ret = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, length, iv_copy, plaintext, ciphertext);
    mbedtls_aes_free(&aes);

    if (ret != 0) {
        result.error = "AES encryption failed";
        return result;
    }

    result.success = true;
    result.encryptedLength = length;
    return result;
}

// Decrypt data using AES-256-CBC
DecryptionResult decryptData(const uint8_t* ciphertext, size_t length, uint8_t* plaintext) {
    DecryptionResult result;

    if (length == 0 || length % 16 != 0) {
        result.error = "Data length must be multiple of 16";
        return result;
    }

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);

    int ret = mbedtls_aes_setkey_dec(&aes, encryptionKey, 256);
    if (ret != 0) {
        result.error = "Failed to set AES key";
        mbedtls_aes_free(&aes);
        return result;
    }

    uint8_t iv_copy[16];
    memcpy(iv_copy, initVector, 16);

    ret = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, length, iv_copy, ciphertext, plaintext);
    mbedtls_aes_free(&aes);

    if (ret != 0) {
        result.error = "AES decryption failed";
        return result;
    }

    result.success = true;
    result.decryptedLength = length;
    return result;
}

// Save encrypted credentials
bool saveCredentials(const char* key, const char* username, const char* password) {
    if (!LittleFS.begin()) return false;

    // Create credentials structure
    struct {
        char username[64];
        char password[64];
    } creds;

    strncpy(creds.username, username, sizeof(creds.username) - 1);
    strncpy(creds.password, password, sizeof(creds.password) - 1);

    // Pad to 128 bytes (multiple of 16)
    uint8_t data[128] = {0};
    memcpy(data, (uint8_t*)&creds, sizeof(creds));

    // Encrypt
    uint8_t encrypted[128];
    EncryptionResult encResult = encryptData(data, 128, encrypted);

    if (!encResult.success) {
        LittleFS.end();
        return false;
    }

    // Save to file
    String filename = "/secure/" + String(key) + ".enc";
    LittleFS.mkdir("/secure");

    File file = LittleFS.open(filename, "w");
    if (!file) {
        LittleFS.end();
        return false;
    }

    file.write(encrypted, 128);
    file.close();

    LittleFS.end();
    return true;
}

// Load encrypted credentials
CredentialsResult loadCredentials(const char* key) {
    CredentialsResult result;

    if (!LittleFS.begin()) return result;

    String filename = "/secure/" + String(key) + ".enc";

    File file = LittleFS.open(filename, "r");
    if (!file) {
        LittleFS.end();
        return result;
    }

    uint8_t encrypted[128] = {0};
    file.read(encrypted, 128);
    file.close();

    // Decrypt
    uint8_t decrypted[128] = {0};
    DecryptionResult decResult = decryptData(encrypted, 128, decrypted);

    if (!decResult.success) {
        LittleFS.end();
        return result;
    }

    // Extract credentials
    struct {
        char username[64];
        char password[64];
    } creds;

    memcpy((uint8_t*)&creds, decrypted, sizeof(creds));

    result.success = true;
    result.username = String(creds.username);
    result.password = String(creds.password);

    LittleFS.end();
    return result;
}

// Securely log events to encrypted file
bool logSecureEvent(const char* event, const char* details) {
    if (!LittleFS.begin()) return false;

    LittleFS.mkdir("/secure_logs");

    File file = LittleFS.open("/secure_logs/events.log", "a");
    if (!file) {
        LittleFS.end();
        return false;
    }

    // Log format: timestamp|event|details|hash
    uint32_t timestamp = millis();
    uint8_t hash[32];

    String logEntry = String(timestamp) + "|" + event + "|" + details + "|";
    calculateSHA256((uint8_t*)logEntry.c_str(), logEntry.length(), hash);

    // Format hash as hex
    char hashStr[65] = {0};
    for (int i = 0; i < 32; i++) {
        snprintf(hashStr + (i * 2), 3, "%02X", hash[i]);
    }

    file.println(logEntry + hashStr);
    file.close();

    LittleFS.end();
    return true;
}

// Rate limiting for suspicious activities
RateLimitStatus checkRateLimit(const char* activity, uint32_t maxPerMinute) {
    static std::map<String, std::vector<uint32_t>> activityTimestamps;

    RateLimitStatus status{true, 0};

    String key(activity);
    uint32_t now = millis();
    uint32_t oneMinuteAgo = now - 60000;

    // Clean old entries
    if (activityTimestamps.find(key) != activityTimestamps.end()) {
        auto& timestamps = activityTimestamps[key];
        timestamps.erase(
            std::remove_if(timestamps.begin(), timestamps.end(),
                         [oneMinuteAgo](uint32_t ts) { return ts < oneMinuteAgo; }),
            timestamps.end()
        );

        status.count = timestamps.size();

        if (status.count >= maxPerMinute) {
            status.allowed = false;
            logSecureEvent("RateLimitExceeded", activity);
            return status;
        }

        timestamps.push_back(now);
    } else {
        activityTimestamps[key].push_back(now);
    }

    return status;
}

// Calculate integrity hash of critical files
bool verifyIntegrity(const char* filename) {
    if (!LittleFS.begin()) return false;

    File file = LittleFS.open(filename, "r");
    if (!file) {
        LittleFS.end();
        return false;
    }

    // Read file content
    std::vector<uint8_t> content;
    while (file.available()) {
        content.push_back(file.read());
    }
    file.close();

    // Calculate hash
    uint8_t hash[32];
    bool hashOk = calculateSHA256(content.data(), content.size(), hash);

    // Read stored hash
    String hashFile = String(filename) + ".hash";
    file = LittleFS.open(hashFile, "r");
    if (!file) {
        LittleFS.end();
        return false;
    }

    uint8_t storedHash[32];
    size_t read = file.read(storedHash, 32);
    file.close();

    LittleFS.end();

    // Compare
    return read == 32 && memcmp(hash, storedHash, 32) == 0;
}

} // namespace SecureStorage
