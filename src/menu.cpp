#include <Arduino.h>
#include <vector>
#include "config.h"
#include <cstddef>
#include "nrf24_tools.h"
#include "subghz.h"
#include "deauth.h"
#include "handshake_capture.h"
#include "beacon_spam.h"
#include "evil_portal.h"
#include "wardriving.h"
#include "badusb.h"
#include "rfid.h"
#include "subghz_replay.h"
#include "ir_learning.h"
#include "ble_jamming.h"
#include "wifi_krack.h"
#include "mifare_bruteforce.h"
#include "ble_fingerprint.h"
#include "dns_spoof.h"
#include "arp_spoof.h"
#include "ssl_strip.h"
#include "ir_bruteforce.h"
#include "ble_relay.h"
#include "exploit_tracker.h"
#include "audio_effects.h"
#include "funny_payloads.h"
#include "settings.h"
#include "default_creds_scanner.h"
#include "wifi_bruteforce.h"
#include "nfc_emulation.h"

enum MenuState {
    MAIN_MENU,
    WIFI_RESULTS,
    WIFI_AP_ACTION,
    BADUSB_MENU,
    BADUSB_OS_SELECT,
    OFFENSIVE_TOOLS_MENU,
    CHAOS_MODE_SCREEN,
    SETTINGS_MENU
};

MenuState g_state = MAIN_MENU;
int g_apActionSel = 0;
int g_badUsbOsSel = 0;
int g_badUsbActionSel = 0;
int g_offensiveToolSel = 0;
int g_settingsSel = 0;
bool g_chaosActive = false;

struct APInfo {
    String bssid;
    uint8_t channel;
};

APInfo ap;

void showResult(const char *title, const String &body, MenuState nextState) {
    g_state = nextState;
}

void runApAction(int idx) {
    switch (idx) {
        case 0: // Deauth this AP
            break;
        case 1: { // Sniff clients
            String body;
            showResult("Clients", body, WIFI_RESULTS);
            break;
        }
        case 2: { // Capture Handshake
            auto result = HandshakeCapture::capture(ap.bssid, ap.channel, 9000);
            String body;
            if (result.eapolFrames == 0) {
                body = "No EAPOL frames seen.\nHold BACK to also\ntrigger a deauth.";
            } else {
                body = String(result.eapolFrames) + " EAPOL frame(s)\nsaved:\n" + result.filePath;
            }
            showResult("Handshake Capture", body, WIFI_RESULTS);
            break;
        }
        case 3: // Back
            g_state = WIFI_RESULTS;
            break;
    }
}

void runBadUsbAction(int osIdx) {
    BadUSB::OSType osType;
    String osName;

    switch (osIdx) {
        case 0:
            osType = BadUSB::OS_WINDOWS;
            osName = "Windows";
            break;
        case 1:
            osType = BadUSB::OS_LINUX;
            osName = "Linux";
            break;
        case 2:
            osType = BadUSB::OS_MACOS;
            osName = "macOS";
            break;
        default:
            return;
    }

    auto result = BadUSB::openWindowsSpam(osType, 15000, 100);
    String body = "Status: " + result.status + "\n" +
                  result.message + "\n" +
                  "Keystrokes: " + String(result.keystrokes);
    String title = "Bad USB [" + osName + "]";
    ExploitTracker::recordBadUSB(result.keystrokes);
    AudioEffects::playSuccessBeep();
    showResult(title.c_str(), body, WIFI_RESULTS);
}

void runChaosMode() {
    if (!Settings::g_config.chaosMode) {
        showResult("Chaos Mode", "Chaos mode is disabled.\nEnable it in Settings.", WIFI_RESULTS);
        return;
    }

    g_chaosActive = true;
    auto result = FunnyPayloads::launchChaosMode();

    String body = "Outils executes: " + String(result.toolsExecuted) + "/11\n\n";
    body += "Resultats:\n" + result.summary;
    body += "\n\nDifficulte trolling: LEVEL 9000!";

    ExploitTracker::displayAchievement(ExploitTracker::CHAOS_MODE_ACTIVATED);
    AudioEffects::playAchievementUnlock();

    showResult("MODE CHAOS", body, WIFI_RESULTS);
}

void runSettingsMenu(int settingIdx) {
    switch (settingIdx) {
        case 0: { // Toggle Audio Effects
            Settings::toggleAudioEffects();
            String status = Settings::g_config.audioEffects ? "ENABLED" : "DISABLED";
            showResult("Audio Effects", status, SETTINGS_MENU);
            break;
        }
        case 1: { // Toggle Achievements
            Settings::toggleAchievements();
            String status = Settings::g_config.achievements ? "ENABLED" : "DISABLED";
            showResult("Achievements", status, SETTINGS_MENU);
            break;
        }
        case 2: { // Toggle Chaos Mode
            Settings::toggleChaosMode();
            String status = Settings::g_config.chaosMode ? "ENABLED" : "DISABLED";
            showResult("Chaos Mode", status, SETTINGS_MENU);
            break;
        }
        case 3: { // Show current settings
            String body = "Audio Effects: " + String(Settings::g_config.audioEffects ? "ON" : "OFF") + "\n";
            body += "Achievements: " + String(Settings::g_config.achievements ? "ON" : "OFF") + "\n";
            body += "Chaos Mode: " + String(Settings::g_config.chaosMode ? "ON" : "OFF") + "\n\n";
            body += "Saved in LittleFS";
            showResult("Current Settings", body, SETTINGS_MENU);
            break;
        }
        case 4: // Back
            g_state = MAIN_MENU;
            return;
    }
}

void runOffensiveTool(int toolIdx) {
    String title;
    String body;

    switch (toolIdx) {
        case 0: { // SubGhz Replay
            auto result = SubGhzReplay::capture(433);
            title = "Sub-GHz Capture";
            body = String(result.success ? "Success" : "Failed") + "\n" +
                   "Frequency: " + String(result.frequency) + " MHz\n" +
                   "Duration: " + String(result.duration) + " ms\n" +
                   "Error: " + result.error;
            ExploitTracker::incrementExploit(result.success);
            if (result.success) AudioEffects::playExploitAlert();
            break;
        }
        case 1: { // IR Learning
            auto result = IRLearning::learn("REMOTE", 30000);
            title = "IR Learning";
            body = String(result.success ? "Code captured" : "Failed") + "\n" +
                   "Code: " + result.code + "\n" +
                   "Duration: " + String(result.durationMs) + " ms";
            break;
        }
        case 2: { // BLE Jamming
            auto result = BLEJamming::startJamming(10000);
            title = "BLE Jamming";
            body = String(result.success ? "Jamming active" : "Failed") + "\n" +
                   "Duration: " + String(result.durationMs) + " ms\n" +
                   "Message: " + result.message;
            break;
        }
        case 3: { // WiFi KRACK
            auto result = WiFiKrack::exploitKrack("TARGET_SSID", 60000);
            title = "WiFi KRACK";
            body = String(result.success ? "Exploit attempted" : "Failed") + "\n" +
                   "Duration: " + String(result.durationMs) + " ms\n" +
                   "Error: " + result.error;
            break;
        }
        case 4: { // Mifare Bruteforce
            auto result = MifareBruteforce::bruteForceKeys(0, 60000);
            title = "Mifare Bruteforce";
            body = String(result.found ? "Key found!" : "No key found") + "\n" +
                   "Attempts: " + String(result.attemptsCount) + "\n" +
                   "Key: " + result.keyFound;
            break;
        }
        case 5: { // BLE Fingerprint
            auto result = BLEFingerprint::scan(30000);
            title = "BLE Fingerprint";
            body = "Devices found: " + String(result.devicesFound) + "\n" +
                   "Duration: " + String(result.durationMs) + " ms\n" +
                   "Error: " + result.error;
            break;
        }
        case 6: { // DNS Spoof
            auto result = DNSSpoof::startSpoof("example.com", "192.168.1.100", 60000);
            title = "DNS Spoof";
            body = String(result.success ? "Spoofing active" : "Failed") + "\n" +
                   "Target: " + result.targetDomain + "\n" +
                   "Error: " + result.error;
            break;
        }
        case 7: { // ARP Spoof
            auto result = ARPSpoof::startSpoof("192.168.1.1", "192.168.1.100", 60000);
            title = "ARP Spoof";
            body = String(result.success ? "Spoofing active" : "Failed") + "\n" +
                   "Packets: " + String(result.packetsSent) + "\n" +
                   "Error: " + result.error;
            break;
        }
        case 8: { // SSL Strip
            auto result = SSLStrip::startMitm(60000);
            title = "SSL Strip";
            body = String(result.success ? "MITM active" : "Failed") + "\n" +
                   "Duration: " + String(result.durationMs) + " ms\n" +
                   "Error: " + result.error;
            break;
        }
        case 9: { // IR Bruteforce
            auto result = IRBruteforce::bruteForce("TV", 30000);
            title = "IR Bruteforce";
            body = String(result.success ? "Code found!" : "Failed") + "\n" +
                   "Attempts: " + String(result.attemptsCount) + "\n" +
                   "Code: " + result.codeFound;
            break;
        }
        case 10: { // BLE Relay
            auto result = BLERelay::startRelay(60000);
            title = "BLE Relay";
            body = String(result.success ? "Relay active" : "Failed") + "\n" +
                   "Duration: " + String(result.durationMs) + " ms\n" +
                   "Error: " + result.error;
            break;
        }
        case 11: { // Default Credentials Scanner
            auto result = DefaultCredScanner::scanDefaultCredentials(60000);
            title = "Default Creds Scanner";
            body = String(result.found ? "Vulnerabilities found!" : "No defaults found") + "\n" +
                   "Devices scanned: " + String(result.devicesScanned) + "\n" +
                   "Credentials tested: " + String(result.credentialsAttempted) + "\n" +
                   "Vulnerable: " + result.vulnerableDevices;
            ExploitTracker::incrementExploit(result.found);
            if (result.found) AudioEffects::playExploitAlert();
            break;
        }
        case 12: { // WiFi Password Brute-Force
            auto result = WiFiBruteforce::bruteForce("TARGET_SSID", 60000);
            title = "WiFi Brute-Force";
            body = String(result.passwordFound ? "Password found!" : "Not in wordlist") + "\n" +
                   "Target: " + result.targetSSID + "\n" +
                   "Attempts: " + String(result.attemptsCount) + "\n" +
                   "Duration: " + String(result.durationMs) + "ms";
            if (result.passwordFound) {
                body += "\nPassword: " + result.foundPassword;
            }
            ExploitTracker::incrementExploit(result.passwordFound);
            if (result.passwordFound) AudioEffects::playExploitAlert();
            break;
        }
        case 13: { // NFC Emulation
            auto testCards = NFCEmulation::getTestCards();
            auto result = NFCEmulation::emulateCard(testCards[0], 30000);
            title = "NFC Emulation";
            body = String(result.success ? "Emulation active" : "Failed") + "\n" +
                   "Card: " + testCards[0].friendlyName + "\n" +
                   "Read count: " + String(result.readCount) + "\n" +
                   "Duration: " + String(result.emulationDurationMs) + "ms";
            ExploitTracker::incrementExploit(result.success && result.readCount > 0);
            break;
        }
        case 14: // Back
            g_state = WIFI_RESULTS;
            return;
    }

    showResult(title.c_str(), body, WIFI_RESULTS);
}

void menuLoop() {
    switch (g_state) {
        case MAIN_MENU: {
            std::vector<String> mainItems = {
                "WiFi Tools", "Bad USB", "RFID", "Offensive Tools"
            };
            if (Settings::g_config.chaosMode) {
                mainItems.push_back("MODE CHAOS!!!");
            }
            mainItems.push_back("Statistics");
            mainItems.push_back("Settings");
            // Main menu navigation
            break;
        }
        case WIFI_AP_ACTION: {
            std::vector<String> actions = {"Deauth this AP", "Sniff clients", "Capture Handshake", "Back"};
            // WiFi AP action menu
            break;
        }
        case BADUSB_OS_SELECT: {
            std::vector<String> osOptions = {"Windows (15k cmd)", "Linux (15k terminal)", "macOS (15k Terminal)", "Back"};
            // OS selection menu - runs selected OS variant
            break;
        }
        case OFFENSIVE_TOOLS_MENU: {
            std::vector<String> offensiveTools = {
                "Sub-GHz Replay", "IR Learning", "BLE Jamming", "WiFi KRACK",
                "Mifare Bruteforce", "BLE Fingerprint", "DNS Spoof", "ARP Spoof",
                "SSL Strip", "IR Bruteforce", "BLE Relay",
                "Default Creds Scanner", "WiFi Brute-Force", "NFC Emulation",
                "Back"
            };
            // Offensive tools menu - calls runOffensiveTool(selection)
            break;
        }
        case CHAOS_MODE_SCREEN: {
            // Chaos mode active - shows live exploit execution
            break;
        }
        case SETTINGS_MENU: {
            std::vector<String> settingsItems = {
                "Toggle Audio Effects",
                "Toggle Achievements",
                "Toggle Chaos Mode",
                "View Current Settings",
                "Back"
            };
            // Settings menu - calls runSettingsMenu(selection)
            break;
        }
        default:
            break;
    }
}
