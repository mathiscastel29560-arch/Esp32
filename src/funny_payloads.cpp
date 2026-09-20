#include "funny_payloads.h"
#include "exploit_tracker.h"
#include "audio_effects.h"
#include "config.h"
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

namespace FunnyPayloads {

std::vector<String> getFunnyBadUSBPayloads() {
    return {
        "powershell -Command \"for($i=0; $i -lt 100; $i++) { Start-Process notepad }\"",
        "cmd /c \"echo VOUS AVEZ ETE HACKED!! & pause\"",
        "powershell -Command \"Start-Process 'https://www.youtube.com/watch?v=dQw4w9WgXcQ'\"",
        "cmd /c \"title SYSTEME COMPROMISE & color 0c & echo WARNING: VIRUS DETECTED!!!\"",
        "powershell -Command \"[System.Windows.Forms.MessageBox]::Show('Vous avez perdu le jeu!', 'GAME OVER')\""
    };
}

std::vector<String> getFunnyWiFiSSIDs() {
    return {
        "FBI_Surveillance_Van",
        "This_Is_A_Virus.exe",
        "5G_Kills_Pigeons",
        "Loading_WiFi_99%",
        "ConnectingPlease_Wait",
        "No_Free_WiFi_Sorry",
        "Hack_Me_If_You_Can",
        "Pretty_Fly_For_WiFi",
        "Abraham_Linksys",
        "Bill_WiFi_Gates"
    };
}

String getFunnyDNSMessage() {
    uint8_t random_msg = millis() % 5;
    switch(random_msg) {
        case 0: return "Redirection vers: ICANHAZWIFI.com (Access Denied!)";
        case 1: return "Domaine verrouille par l'FBI (blague)";
        case 2: return "Vous avez ete redirige vers: LOCALHOST:666";
        case 3: return "Serveur DNS: 127.0.0.1 (c'est vous!)";
        case 4: return "Message d'erreur mystique: BEEP_BOOP_PWNED";
        default: return "???";
    }
}

String getFunnyRFIDMessage() {
    uint8_t random_msg = millis() % 4;
    switch(random_msg) {
        case 0: return "Clone reussi! Vous etes maintenant directeur general.";
        case 1: return "Authentification acceptee (blague - acces refusee!)";
        case 2: return "Badge clone: EMPLOYEE_OF_THE_MONTH";
        case 3: return "Clonage en cours... OPERATION REUSSIE (peut-etre)";
        default: return "???";
    }
}

String getFunnyBLEMessage() {
    uint8_t random_msg = millis() % 3;
    switch(random_msg) {
        case 0: return "Appareil Bluetooth detecte a 500km de distance!";
        case 1: return "Votre montre connectee a ete relayee par satellite";
        case 2: return "Commandes Bluetooth interceptees: Volume +9000!";
        default: return "???";
    }
}

ChaosResult launchChaosMode() {
    #if !ENABLE_CHAOS_MODE
    return {false, 0, "Chaos mode disabled in config"};
    #endif

    AudioEffects::playChaosMode();
    ExploitTracker::unlockAchievement(ExploitTracker::CHAOS_MODE_ACTIVATED);

    ChaosResult result{true, 0, ""};

    Serial.println("\n\n=== MODE CHAOS ACTIVÉ ===");
    Serial.println("Tous les outils en simultané!!!\n");

    // Lancer tous les 11 outils en même temps
    // 0: SubGhz Replay
    auto r1 = SubGhzReplay::capture(433);
    result.toolsExecuted++;
    result.summary += "SubGHz: " + String(r1.success ? "OK" : "FAIL") + " | ";

    // 1: IR Learning
    auto r2 = IRLearning::learn("REMOTE", 5000);
    result.toolsExecuted++;
    result.summary += "IR: " + String(r2.success ? "OK" : "FAIL") + " | ";

    // 2: BLE Jamming
    auto r3 = BLEJamming::startJamming(5000);
    result.toolsExecuted++;
    result.summary += "BLE: " + String(r3.success ? "OK" : "FAIL") + " | ";

    // 3: WiFi KRACK
    auto r4 = WiFiKrack::exploitKrack("CHAOS", 5000);
    result.toolsExecuted++;
    result.summary += "KRACK: " + String(r4.success ? "OK" : "FAIL") + " | ";

    // 4: Mifare Bruteforce
    auto r5 = MifareBruteforce::bruteForceKeys(0, 5000);
    result.toolsExecuted++;
    result.summary += "Mifare: " + String(r5.found ? "OK" : "FAIL") + " | ";

    // 5: BLE Fingerprint
    auto r6 = BLEFingerprint::scan(5000);
    result.toolsExecuted++;
    result.summary += "BLEFp: " + String(r6.devicesFound > 0 ? "OK" : "FAIL") + " | ";

    // 6: DNS Spoof
    auto r7 = DNSSpoof::startSpoof("chaos.com", "1.2.3.4", 5000);
    result.toolsExecuted++;
    result.summary += "DNS: " + String(r7.success ? "OK" : "FAIL") + " | ";

    // 7: ARP Spoof
    auto r8 = ARPSpoof::startSpoof("192.168.1.1", "192.168.1.100", 5000);
    result.toolsExecuted++;
    result.summary += "ARP: " + String(r8.success ? "OK" : "FAIL") + " | ";

    // 8: SSL Strip
    auto r9 = SSLStrip::startMitm(5000);
    result.toolsExecuted++;
    result.summary += "SSL: " + String(r9.success ? "OK" : "FAIL") + " | ";

    // 9: IR Bruteforce
    auto r10 = IRBruteforce::bruteForce("TV", 5000);
    result.toolsExecuted++;
    result.summary += "IRBf: " + String(r10.success ? "OK" : "FAIL") + " | ";

    // 10: BLE Relay
    auto r11 = BLERelay::startRelay(5000);
    result.toolsExecuted++;
    result.summary += "Relay: " + String(r11.success ? "OK" : "FAIL");

    ExploitTracker::incrementExploit(result.success);

    Serial.println("=== CHAOS COMPLETE ===");
    Serial.println("Outils executes: " + String(result.toolsExecuted));
    Serial.println("Resultat: " + result.summary);
    Serial.println("======================\n");

    return result;
}

} // namespace FunnyPayloads
