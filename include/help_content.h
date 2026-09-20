#pragma once
#include <Arduino.h>

// Static reference text for the on-device "Aide" menu (menu.cpp): what
// each feature does, how to trigger it, and how to read its result.
// Deliberately short per topic -- the OLED path only has a handful of
// visible rows (see ui/oled_ui.cpp), so anything longer just gets
// clipped there the same way any other result screen would.
namespace HelpContent {

struct Topic {
    const char *title;
    const char *body;
};

static const Topic TOPICS[] = {
    {"Scan Wi-Fi",
     "Liste les reseaux a portee,\n"
     "tries par signal.\n"
     "Selectionne-en un puis OK:\n"
     "Deauth cible ou Sniff clients.\n"
     "Resultat: SSID, RSSI, chiffrement."},
    {"Wi-Fi: Deauth / Sniff",
     "Deauth: deconnecte les appareils\n"
     "d'un AP choisi. Necessite RETOUR\n"
     "maintenu + OK (verrou TX).\n"
     "Sniff clients: liste les MAC vues\n"
     "sur ce reseau, 4s d'ecoute."},
    {"Scan BLE",
     "Liste les appareils Bluetooth LE\n"
     "a portee (5s d'ecoute).\n"
     "Selectionne-en un pour: Audit\n"
     "GATT ou Fuzz.\n"
     "Resultat: nom/adresse, RSSI,\n"
     "fabricant deduit."},
    {"BLE: Audit GATT / Fuzz",
     "Audit: connexion, liste les\n"
     "services/caracteristiques. Dit\n"
     "si lisible sans appairage,\n"
     "ecrivable sans auth, pairing\n"
     "Just Works (pas de MITM).\n"
     "Fuzz: ecritures test + reconnexions\n"
     "rapides. A faire en isolation!"},
    {"Scan 2.4GHz (NRF24)",
     "Balaie 20 canaux 2.4GHz, mesure\n"
     "l'activite radio sur chacun.\n"
     "Resultat: canal le plus charge\n"
     "+ nombre de detections."},
    {"Sub-GHz (CC1101, 433MHz)",
     "Scan: niveau RSSI sur 433.92MHz.\n"
     "Record: capture 5s de signal\n"
     "(telecommandes, capteurs...).\n"
     "Replay: renvoie la derniere\n"
     "capture. Necessite RETOUR maintenu."},
    {"Beacon Spam",
     "Diffuse de faux points d'acces\n"
     "Wi-Fi (SSID de test, voir\n"
     "config.h: DEFAULT_BEACON_SSIDS).\n"
     "Demarrage necessite RETOUR\n"
     "maintenu. Rebascule start/stop."},
    {"Faux portail (Evil Portal)",
     "Cree un point d'acces + page de\n"
     "connexion factice.\n"
     "Identifiants saisis loggues\n"
     "localement (logs/).\n"
     "Demarrage necessite RETOUR\n"
     "maintenu. Rebascule start/stop."},
    {"IR (telecommande)",
     "Toggle TV: envoie une liste de\n"
     "codes courants (best-effort).\n"
     "Apprendre: capture le prochain\n"
     "bouton presse (5s d'ecoute).\n"
     "Rejouer: renvoie la derniere\n"
     "capture apprise."},
    {"Surveillance BLE-spam",
     "Detecte passivement un flood\n"
     "d'appairage BLE autour de toi.\n"
     "N'emet rien, alerte juste si le\n"
     "seuil est depasse (config.h).\n"
     "Check Alert: montre la derniere\n"
     "alerte, sinon rien a signaler."},
    {"Wardrive Snapshot",
     "Ajoute une ligne au log CSV:\n"
     "position GPS + reseaux vus.\n"
     "Fichier: logs/wardrive.csv\n"
     "(accessible via le panneau web)."},
    {"Verrou TX (RETOUR)",
     "Les actions radio actives (deauth,\n"
     "spam, replay sub-GHz...) exigent\n"
     "de maintenir RETOUR au moment\n"
     "d'appuyer sur OK.\n"
     "Sans ca: 'Blocked: hold BACK'."},
    {"Dual-boot / Boot into Bruce",
     "Redemarre sur Bruce, si flashe\n"
     "sur ota_1 (voir bruce-board/).\n"
     "Un simple cycle d'alimentation\n"
     "revient sur ce firmware.\n"
     "Si jamais flashe: 'Bruce not\n"
     "flashed to ota_1'."},
};

constexpr size_t TOPIC_COUNT = sizeof(TOPICS) / sizeof(TOPICS[0]);

} // namespace HelpContent
