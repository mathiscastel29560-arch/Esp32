#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <Arduino.h>
#include <cstdint>
#include <cstring>

enum class ErrorCode : uint8_t {
    // Success codes
    SUCCESS = 0,

    // Hardware errors
    HW_NOT_INITIALIZED = 10,
    HW_COMMUNICATION_FAILED = 11,
    HW_TIMEOUT = 12,
    HW_RESOURCE_BUSY = 13,

    // Input validation errors
    INVALID_BSSID = 20,
    INVALID_SSID = 21,
    INVALID_CHANNEL = 22,
    INVALID_MAC_ADDRESS = 23,
    INVALID_PARAMETER = 24,

    // Operation errors
    TX_NOT_ARMED = 30,
    OPERATION_TIMEOUT = 31,
    INSUFFICIENT_MEMORY = 32,
    OPERATION_CANCELLED = 33,

    // Cryptography errors
    CRYPTO_FAILED = 40,
    PBKDF2_FAILED = 41,
    HMAC_FAILED = 42,

    // Network errors
    NO_NETWORK_FOUND = 50,
    CONNECTION_FAILED = 51,
    HANDSHAKE_TIMEOUT = 52,

    // Unknown error
    UNKNOWN_ERROR = 255
};

class ErrorHandler {
public:
    static ErrorHandler& instance() {
        static ErrorHandler eh;
        return eh;
    }

    // Log error with context
    void logError(const char* module, ErrorCode code, const char* message = "") {
        printf("[ERROR] %s (0x%02X): %s\n", module, (uint8_t)code, message);
        lastError = code;
        lastModule = module;
    }

    // Log warning
    void logWarning(const char* module, const char* message) {
        printf("[WARN] %s: %s\n", module, message);
    }

    // Log info
    void logInfo(const char* module, const char* message) {
        printf("[INFO] %s: %s\n", module, message);
    }

    // Get last error
    ErrorCode getLastError() const { return lastError; }
    const char* getLastModule() const { return lastModule; }

    // Convert error to human-readable string
    static const char* toString(ErrorCode code) {
        switch(code) {
            case ErrorCode::SUCCESS: return "Success";
            case ErrorCode::HW_NOT_INITIALIZED: return "Hardware not initialized";
            case ErrorCode::HW_COMMUNICATION_FAILED: return "Hardware communication failed";
            case ErrorCode::HW_TIMEOUT: return "Hardware timeout";
            case ErrorCode::INVALID_BSSID: return "Invalid BSSID format";
            case ErrorCode::INVALID_SSID: return "Invalid SSID";
            case ErrorCode::INVALID_CHANNEL: return "Invalid WiFi channel";
            case ErrorCode::TX_NOT_ARMED: return "TX not armed - hold BACK button";
            case ErrorCode::OPERATION_TIMEOUT: return "Operation timeout";
            case ErrorCode::INSUFFICIENT_MEMORY: return "Insufficient memory";
            case ErrorCode::PBKDF2_FAILED: return "PBKDF2 key derivation failed";
            case ErrorCode::HANDSHAKE_TIMEOUT: return "Handshake capture timeout";
            default: return "Unknown error";
        }
    }

    // Check if operation should continue
    bool isRecoverable(ErrorCode code) {
        switch(code) {
            case ErrorCode::HW_TIMEOUT:
            case ErrorCode::OPERATION_TIMEOUT:
            case ErrorCode::HANDSHAKE_TIMEOUT:
            case ErrorCode::HW_RESOURCE_BUSY:
                return true;  // Can retry

            default:
                return false;  // Fatal error
        }
    }

private:
    ErrorCode lastError = ErrorCode::SUCCESS;
    const char* lastModule = "UNKNOWN";
};

#define LOG_ERROR(module, code, msg) ErrorHandler::instance().logError(module, code, msg)
#define LOG_WARN(module, msg) ErrorHandler::instance().logWarning(module, msg)
#define LOG_INFO(module, msg) ErrorHandler::instance().logInfo(module, msg)

#endif
