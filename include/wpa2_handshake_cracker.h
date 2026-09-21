#pragma once
#include <Arduino.h>

namespace WPA2HandshakeCracker {

struct CrackResult {
    bool success;
    bool passwordFound;
    String ssid;
    String password;
    uint32_t attemptsCount;
};

// Capture WPA2 handshake and attempt dictionary attack
CrackResult captureAndCrack(const String &targetSSID, uint32_t timeoutMs = 30000);

// Dictionary attack on captured handshake
CrackResult dictionaryAttack(const String &ssid, uint32_t attemptLimit = 1000);

}  // namespace WPA2HandshakeCracker
