#include <Arduino.h>
#include <vector>
#include "config.h"
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

enum MenuState {
    WIFI_RESULTS,
    WIFI_AP_ACTION,
    BADUSB_MENU,
    BADUSB_OS_SELECT
};

MenuState g_state = WIFI_RESULTS;
int g_apActionSel = 0;
int g_badUsbOsSel = 0;
int g_badUsbActionSel = 0;

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
    showResult(title.c_str(), body, WIFI_RESULTS);
}

void runRfidScan() {
    auto result = RFID::scan();
    String body;

    if (result.found) {
        String uidHex;
        for (int i = 0; i < result.tag.uidLen; i++) {
            if (result.tag.uid[i] < 0x10) uidHex += "0";
            uidHex += String(result.tag.uid[i], HEX);
        }
        body = "Tag found!\n";
        body += "UID: " + uidHex + "\n";
        body += "Type: " + result.tag.type + "\n";
        body += "Capacity: " + String(result.tag.capacity) + " bytes\n";
        body += "Sectors: " + String(result.tag.sectorCount);
    } else {
        body = "No tag found.\nTimeout: 5 seconds.\n";
        body += "Place tag near reader.";
    }
    body += "\nScan time: " + String(result.readTimeMs) + "ms";

    showResult("RFID Scan", body, WIFI_RESULTS);
}

void runRfidClone() {
    auto result = RFID::scan();

    String body;
    if (result.found) {
        auto cloneResult = RFID::clone(result.tag);
        if (cloneResult.success) {
            body = "Clone successful!\n";
            body += "Source: " + cloneResult.sourceUid + "\n";
            body += "Target: " + cloneResult.targetUid + "\n";
            body += "Bytes written: " + String(cloneResult.bytesWritten);
        } else {
            body = "Clone failed: " + cloneResult.error;
        }
    } else {
        body = "No source tag found.\nScan first.";
    }

    showResult("RFID Clone", body, WIFI_RESULTS);
}

void menuLoop() {
    switch (g_state) {
        case WIFI_AP_ACTION: {
            std::vector<String> actions = {"Deauth this AP", "Sniff clients", "Capture Handshake", "Back"};
            // menu logic here
            break;
        }
        case BADUSB_OS_SELECT: {
            std::vector<String> osOptions = {"Windows (15k cmd)", "Linux (15k terminal)", "macOS (15k Terminal)", "Back"};
            // OS selection menu - in real implementation would show on OLED
            break;
        }
        default:
            break;
    }
}
