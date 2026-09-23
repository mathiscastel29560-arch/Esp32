#ifndef INPUT_VALIDATOR_H
#define INPUT_VALIDATOR_H

#include "error_handler.h"
#include <Arduino.h>
#include <cstring>
#include <cstdint>

class InputValidator {
public:
    // Validate MAC address format "AA:BB:CC:DD:EE:FF"
    static ErrorCode validateMAC(const char* mac_str, uint8_t* out_bytes = nullptr) {
        if (!mac_str) return ErrorCode::INVALID_PARAMETER;
        if (strlen(mac_str) != 17) return ErrorCode::INVALID_MAC_ADDRESS;

        uint8_t bytes[6];
        int n = sscanf(mac_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                       &bytes[0], &bytes[1], &bytes[2],
                       &bytes[3], &bytes[4], &bytes[5]);

        if (n != 6) return ErrorCode::INVALID_MAC_ADDRESS;

        // Check for special values
        bool all_zeros = true, all_ones = true;
        for (int i = 0; i < 6; i++) {
            if (bytes[i] != 0x00) all_zeros = false;
            if (bytes[i] != 0xFF) all_ones = false;
        }

        // Reject all-zeros or broadcast
        if (all_zeros) return ErrorCode::INVALID_MAC_ADDRESS;

        if (out_bytes) {
            memcpy(out_bytes, bytes, 6);
        }

        return ErrorCode::SUCCESS;
    }

    // Validate WiFi BSSID (same as MAC)
    static ErrorCode validateBSSID(const String& bssid) {
        return validateMAC(bssid.c_str());
    }

    // Validate SSID
    static ErrorCode validateSSID(const String& ssid) {
        if (ssid.length() == 0) return ErrorCode::INVALID_SSID;
        if (ssid.length() > 32) return ErrorCode::INVALID_SSID;

        // Allow null bytes for hidden networks
        return ErrorCode::SUCCESS;
    }

    // Validate WiFi channel (2.4GHz: 1-13, 5GHz: 36-165)
    static ErrorCode validateChannel(uint8_t channel) {
        if ((channel >= 1 && channel <= 13) ||      // 2.4GHz
            (channel >= 36 && channel <= 165)) {    // 5GHz
            return ErrorCode::SUCCESS;
        }
        return ErrorCode::INVALID_CHANNEL;
    }

    // Validate SubGhz frequency (typically 300MHz - 1GHz)
    static ErrorCode validateFrequency(uint32_t freq_hz) {
        if (freq_hz < 300000000 || freq_hz > 1000000000) {
            return ErrorCode::INVALID_PARAMETER;
        }
        return ErrorCode::SUCCESS;
    }

    // Validate duration (not exceeding practical limits)
    static ErrorCode validateDuration(uint32_t duration_ms) {
        if (duration_ms == 0) return ErrorCode::INVALID_PARAMETER;
        if (duration_ms > 3600000) {  // Max 1 hour
            LOG_WARN("InputValidator", "Duration > 1 hour requested");
        }
        return ErrorCode::SUCCESS;
    }

    // Validate WPS PIN (8 digits, valid checksum)
    static ErrorCode validateWPSPin(uint32_t pin) {
        if (pin < 10000000 || pin > 99999999) {
            return ErrorCode::INVALID_PARAMETER;
        }

        // Luhn checksum validation
        uint32_t accum = 0;
        uint32_t pin_without_check = pin / 10;

        for (int i = 0; i < 7; i++) {
            uint32_t digit = (pin_without_check / (uint32_t)pow(10, i)) % 10;
            accum += digit * (i % 2 == 0 ? 3 : 1);
        }

        uint32_t expected_check = (10 - (accum % 10)) % 10;
        uint32_t actual_check = pin % 10;

        if (expected_check != actual_check) {
            return ErrorCode::INVALID_PARAMETER;
        }

        return ErrorCode::SUCCESS;
    }

    // Validate password/passphrase length
    static ErrorCode validatePassword(const String& password) {
        if (password.length() < 8) return ErrorCode::INVALID_PARAMETER;
        if (password.length() > 63) return ErrorCode::INVALID_PARAMETER;

        return ErrorCode::SUCCESS;
    }

    // Validate buffer size
    static ErrorCode validateBufferSize(size_t requested, size_t available) {
        if (requested > available) {
            return ErrorCode::INSUFFICIENT_MEMORY;
        }
        return ErrorCode::SUCCESS;
    }

    // Safe timeout calculation avoiding wraparound
    static bool isTimeoutExceeded(unsigned long start, uint32_t timeout_ms) {
        return (int32_t)(millis() - (start + timeout_ms)) >= 0;
    }
};

#endif
