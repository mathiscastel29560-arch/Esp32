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

enum MenuState {
    WIFI_RESULTS,
    WIFI_AP_ACTION
};

MenuState g_state = WIFI_RESULTS;
int g_apActionSel = 0;

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

void menuLoop() {
    switch (g_state) {
        case WIFI_AP_ACTION: {
            std::vector<String> actions = {"Deauth this AP", "Sniff clients", "Capture Handshake", "Back"};
            // menu logic here
            break;
        }
        default:
            break;
    }
}
