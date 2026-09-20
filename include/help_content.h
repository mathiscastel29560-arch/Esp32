#pragma once
#include <Arduino.h>

// Comprehensive help system for the on-device menu
// Topics organized by category with detailed explanations
namespace HelpContent {

struct Topic {
    const char *category;  // WiFi, BLE, RF, Tools, System, Help
    const char *icon;      // Visual indicator
    const char *title;     // Short title
    const char *body;      // Detailed explanation
};

// Organized by category for better navigation
static const Topic TOPICS[] = {
    // ========== WIFI CATEGORY ==========
    {"WiFi", "W", "Scan Networks",
     "Wi-Fi Scanner (5 seconds):\n"
     "Liste tous les reseaux a portee,\n"
     "tries par signal (RSSI).\n"
     "\n"
     "Pour chaque reseau: SSID, BSSID,\n"
     "canal, chiffrement (OPEN/WEP/WPA/WPA2).\n"
     "\n"
     "Selection + OK: Deauth ou Sniff.\n"
     "- Deauth: force deconnexion d'appareils\n"
     "- Sniff: ecoute passive des clients\n"
     "\n"
     "TX lock: Deauth/Evil Portal/Beacon\n"
     "REQUIERT RETOUR maintenu."},

    {"WiFi", "W", "Deauth Attack",
     "802.11 Deauthentication Frames:\n"
     "\n"
     "Force les clients a se deconnecter\n"
     "d'un AP en envoyant des frames de\n"
     "deconnexion spoofees.\n"
     "\n"
     "SECURITE: Necessite interrupteur TX\n"
     "(maintenir RETOUR) pour activer.\n"
     "\n"
     "Resultat: nombre de frames envoyes,\n"
     "duree d'execution.\n"
     "\n"
     "ATTENTION: Illegal sans autorisation!\n"
     "Usage = tests sur VOtre propre reseau."},

    {"WiFi", "W", "Beacon Spam",
     "Fake Access Point Broadcasting:\n"
     "\n"
     "Diffuse de faux points d'acces Wi-Fi\n"
     "avec des SSID test (configurable).\n"
     "\n"
     "A porter dans la rue = 'wardriving'.\n"
     "Le reseau n'a pas vraiment d'internet,\n"
     "ce sont juste des beacons.\n"
     "\n"
     "TX lock: RETOUR maintenu requis.\n"
     "Start/Stop: appuyer OK toggle etat.\n"
     "\n"
     "Resultat: nombre de beacons/sec,\n"
     "canaux utilises."},

    {"WiFi", "W", "Evil Portal",
     "Captive Portal (Fake Hotspot):\n"
     "\n"
     "Cree un faux point d'acces libre\n"
     "('Free-WiFi') avec page de connexion.\n"
     "\n"
     "Utilisateurs se connectent = leur\n"
     "donnees sont loggues.\n"
     "\n"
     "Logs saved: /logs/portal_submissions.csv\n"
     "Format: timestamp, SSID, identifiant.\n"
     "\n"
     "TX lock: RETOUR maintenu requis.\n"
     "Web panel: acces au contenu.\n"
     "\n"
     "LEGAL WARNING: Fraud is illegal!"},

    {"WiFi", "W", "Wardriving",
     "GPS-Tagged Network Logging:\n"
     "\n"
     "Ajoute UNE ligne au fichier CSV:\n"
     "position GPS + tous les reseaux vus.\n"
     "\n"
     "Fichier: /logs/wardrive.csv\n"
     "Columns: timestamp, SSID, BSSID,\n"
     "channel, RSSI, encryption, lat, lon.\n"
     "\n"
     "Snapshot = 1 ligne = 1 position GPS.\n"
     "Relancer snapshot = nouvelle ligne.\n"
     "\n"
     "Use case: mapping all WiFi networks\n"
     "in a large area over time."},

    // ========== BLE CATEGORY ==========
    {"BLE", "B", "Scan Devices",
     "Bluetooth Low Energy Inventory (5s):\n"
     "\n"
     "Ecoute passive, liste les appareils\n"
     "BLE a portee.\n"
     "\n"
     "Pour chaque appareil:\n"
     "- Adresse MAC\n"
     "- RSSI (proximite)\n"
     "- Nom (si disponible)\n"
     "- Fabricant (deduit de OUI)\n"
     "\n"
     "Selection + OK: Audit GATT ou Fuzz.\n"
     "\n"
     "Ne demande PAS d'appairage."},

    {"BLE", "B", "GATT Audit",
     "Service/Characteristic Analysis:\n"
     "\n"
     "Se connecte au device, enumere tous\n"
     "les services et caracteristiques.\n"
     "\n"
     "Pour chaque characteristic, test:\n"
     "- Peut-on lire sans appairage?\n"
     "- Peut-on ecrire sans authentification?\n"
     "- Pairing Just Works (pas MITM)?\n"
     "- Fuites de Device Info?\n"
     "\n"
     "Resultat = vulnerabilites potentielles.\n"
     "\n"
     "Use case: Audit securite BLE,\n"
     "detection de misconfigurations."},

    {"BLE", "B", "BLE Fuzzing",
     "Malformed Data Testing:\n"
     "\n"
     "Se connecte + tente ecritures\n"
     "invalides sur les characteristics:\n"
     "- Donnees oversized\n"
     "- Formats inattendus\n"
     "- Reconnexions rapides\n"
     "\n"
     "Objectif: chercher crashes/hangs.\n"
     "\n"
     "ATTENTION: A faire en isolation!\n"
     "Peut causer instabilite temporaire\n"
     "ou briques le device.\n"
     "\n"
     "Use case: Testing robustness."},

    {"BLE", "B", "Spam Detection",
     "Passive BLE Pairing Flood Monitor:\n"
     "\n"
     "Ecoute passive. N'emet RIEN.\n"
     "\n"
     "Detecte si quelqu'un nearby fait\n"
     "un flood de beacons d'appairage:\n"
     "- Apple Continuity\n"
     "- Google Fast Pair\n"
     "- Microsoft Swift Pair\n"
     "\n"
     "Check Alert: montre derniere alerte.\n"
     "\n"
     "Seuil configurable: config.h\n"
     "BLE_SPAM_DISTINCT_MAC_THRESHOLD"},

    // ========== RF CATEGORY ==========
    {"RF", "R", "2.4 GHz Spectrum",
     "NRF24 Carrier Detection Scanner:\n"
     "\n"
     "Balaie tous les canaux 2.4 GHz,\n"
     "mesure l'activite radio.\n"
     "\n"
     "Receive-only: ne transmet RIEN.\n"
     "Utilise detection de porteur NRF24L01+.\n"
     "\n"
     "Pour chaque canal (0-125):\n"
     "Nombre de fois 'signal actif' detecte\n"
     "(0-255 = niveau d'activite).\n"
     "\n"
     "Resultat: canal le plus charge.\n"
     "\n"
     "Use case: trouver canaux surcharges\n"
     "avant de deployer WiFi/mesh."},

    {"RF", "R", "Sub-GHz Scan (433MHz)",
     "CC1101 Radio Frequency Scanning:\n"
     "\n"
     "Mesure niveau RSSI sur 433.92 MHz\n"
     "(bande sub-GHz commune).\n"
     "\n"
     "Use case: chercher telecommandes,\n"
     "capteurs wireless nearby.\n"
     "\n"
     "Receive-only: ne transmet RIEN.\n"
     "\n"
     "RSSI = signal strength (-95 a -20 dBm).\n"
     "Plus proche de 0 = plus fort signal."},

    {"RF", "R", "Sub-GHz Record",
     "OOK/ASK Signal Capture (433MHz):\n"
     "\n"
     "Enregistre 5 secondes de signal\n"
     "sur 433.92 MHz en format OOK/ASK\n"
     "(telecommandes, capteurs, etc).\n"
     "\n"
     "Capture: pulses durees (mark/space).\n"
     "Stocke: /logs/subghz_captures.txt\n"
     "\n"
     "Prerequisite: hardware CC1101 wired.\n"
     "\n"
     "Utilisateur: appuie sur bouton\n"
     "de la telecommande pendant capture."},

    {"RF", "R", "Sub-GHz Replay",
     "Recorded Signal Retransmission:\n"
     "\n"
     "Renvoie la derniere capture\n"
     "enregistree (si valide).\n"
     "\n"
     "TX lock: RETOUR maintenu REQUIS.\n"
     "\n"
     "WARNING: Peut causer interferences!\n"
     "- Portes de garage\n"
     "- Eclairage wireless\n"
     "- Serrures intelligentes\n"
     "\n"
     "Use case: testing own remotes on\n"
     "devices you own (garage door, etc)."},

    {"RF", "R", "IR: TV Toggle",
     "Infrared Power Command Transmission:\n"
     "\n"
     "Envoie une serie de codes IR\n"
     "pouvoir TV courants (best-effort).\n"
     "\n"
     "Utilise GPIO 38 (IR LED).\n"
     "\n"
     "Resultat: 'OK' ou 'No IR device'.\n"
     "\n"
     "Pour tester: pointer ESP vers\n"
     "telecommande officielle TV,\n"
     "verifier si codes matchent."},

    {"RF", "R", "IR: Learn Code",
     "Infrared Signal Capture & Storage:\n"
     "\n"
     "Ecoute pendant 5 secondes sur\n"
     "le recepteur IR (GPIO 39).\n"
     "\n"
     "Utilisateur: appuie sur bouton\n"
     "telecommande pointee vers l'ESP.\n"
     "\n"
     "Sauvegarde: /logs/ir_codes.txt\n"
     "Format: durees des pulses (µs).\n"
     "\n"
     "Permet construction d'une base\n"
     "de codes pour tests."},

    {"RF", "R", "IR: Replay Learned",
     "Retransmit Last Learned IR Code:\n"
     "\n"
     "Renvoie le dernier code IR\n"
     "enregistre (si valide).\n"
     "\n"
     "Utilisateur: pointer ESP vers\n"
     "appareil IR cible (TV, AC, etc).\n"
     "\n"
     "Resultat: 'OK' ou 'No code'."},

    // ========== SYSTEM CATEGORY ==========
    {"System", "S", "TX Arm (Safety Lock)",
     "Radio Transmission Safety Interlock:\n"
     "\n"
     "TOUS les emissions RF (deauth,\n"
     "beacon, portal, sub-GHz replay, IR)\n"
     "REQUIERENT maintenir RETOUR.\n"
     "\n"
     "Raison: prevenir accidents.\n"
     "\n"
     "Workflow:\n"
     "1. Selection: D-Pad UP/DOWN\n"
     "2. Action: RETOUR + OK (simultane)\n"
     "3. Relacher RETOUR = execution\n"
     "\n"
     "Status: 'Armed' ou 'Locked'.\n"
     "\n"
     "Auto-disarm: timeout 10s."},

    {"System", "S", "GPS Map",
     "France Outline + Live Position:\n"
     "\n"
     "Affiche contour simplifie France\n"
     "avec dot a ta position GPS.\n"
     "\n"
     "Update: chaque seconde si fix OK.\n"
     "\n"
     "Sans fix GPS: 'No GPS fix'.\n"
     "\n"
     "Requires: GPS module (NEO-6M) wired\n"
     "+ initialised.\n"
     "\n"
     "Use case: verify GPS is working,\n"
     "rough positioning check."},

    {"System", "S", "Battery Status",
     "LiPo Voltage & Percentage Display:\n"
     "\n"
     "Affiche dans le header:\n"
     "- Voltage (V)\n"
     "- Percentage (0-100%)\n"
     "\n"
     "Requires: Battery ADC (GPIO 7) + \n"
     "voltage divider wired correctly.\n"
     "\n"
     "Update: continu (chaque frame).\n"
     "\n"
     "Used by: wardriving, any long op.\n"
     "\n"
     "Tip: charge avant field use!"},

    {"System", "S", "Dual-boot (OTA0/OTA1)",
     "Boot ESP32-DIV Alternate Firmware:\n"
     "\n"
     "Redemarre sur OTA1 (ESP32-DIV)\n"
     "si flashe au prealable.\n"
     "\n"
     "OTA0 = ce firmware (audit tool)\n"
     "OTA1 = ESP32-DIV (alternate OS)\n"
     "\n"
     "Rollback: auto-valide si boot OK.\n"
     "\n"
     "WARNING: ESP32-DIV n'a pas de\n"
     "rollback = recovery manuelle if crash.\n"
     "\n"
     "Check: 'Booting OTA1...'"},

    {"System", "S", "Settings",
     "Runtime Configuration Flags:\n"
     "\n"
     "Toggles persistants via LittleFS:\n"
     "- Audio Effects (beeps/alerts)\n"
     "- Achievements (unlock tracking)\n"
     "- Chaos Mode (all tools simultane)\n"
     "\n"
     "Saved: /config/settings.json\n"
     "\n"
     "Changes survive reboot.\n"
     "\n"
     "Web panel: modify remotely.\n"
     "On-device: coming soon."},

    // ========== ABOUT CATEGORY ==========
    {"Help", "?", "About This Device",
     "ESP32-S3 Multi-RF Audit Tool:\n"
     "\n"
     "Hardware: ESP32-S3-N16R8\n"
     "Flash: 16 MB\n"
     "PSRAM: 8 MB\n"
     "CPU: 240 MHz dual-core\n"
     "\n"
     "Radios: WiFi 802.11 + BLE + CC1101\n"
     "(Sub-GHz) + NRF24L01+ (2.4GHz) +\n"
     "IR Rx/Tx\n"
     "\n"
     "Display: TFT ILI9341 or OLED\n"
     "(auto-detected)\n"
     "\n"
     "Sensors: GPS, RTC, Battery ADC.\n"
     "\n"
     "Web Panel: port 8080"},

    {"Help", "?", "Storage Locations",
     "LittleFS File Paths:\n"
     "\n"
     "Config:\n"
     "/config/settings.json = runtime flags\n"
     "\n"
     "Logs:\n"
     "/logs/wardrive.csv = GPS networks\n"
     "/logs/portal_submissions.csv = creds\n"
     "/logs/ir_codes.txt = learned IR\n"
     "/logs/subghz/captures.txt = RF\n"
     "\n"
     "Clones:\n"
     "/rfid_clones/*.json = MIFARE data\n"
     "\n"
     "Access via: web panel (port 8080)\n"
     "or USB mount (mass storage mode)."},

    {"Help", "?", "Legal Notice",
     "AUTHORIZED USE ONLY:\n"
     "\n"
     "This tool is for security audits\n"
     "of YOUR OWN devices/networks.\n"
     "\n"
     "Illegal uses:\n"
     "- Jamming licensed frequencies\n"
     "- Targeting networks without consent\n"
     "- Intercepting private data\n"
     "- Wire fraud (evil portal)\n"
     "\n"
     "User assumes ALL LEGAL LIABILITY.\n"
     "Encrypt, test, audit responsibly.\n"
     "\n"
     "Check local RF regulations!"},
};

constexpr size_t TOPIC_COUNT = sizeof(TOPICS) / sizeof(TOPICS[0]);

}  // namespace HelpContent
