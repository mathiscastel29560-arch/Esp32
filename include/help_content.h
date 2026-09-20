#pragma once

struct Topic {
    const char *title;
    const char *text;
};

static const Topic TOPICS[] = {
    {"Capture Handshake",
     "Depuis un reseau selectionne:\n"
     "ecoute passive 9s des trames\n"
     "EAPOL (poignee de main WPA2).\n"
     "Maintiens RETOUR pour aussi\n"
     "forcer un deauth (reconnexion\n"
     "= nouvelle poignee de main).\n"
     "Sauve un .pcap dans /logs/\n"
     "handshakes -- cassage hors\n"
     "ligne (hashcat/aircrack) sur\n"
     "un ordinateur, pas ici."},
    {"Scan BLE",
     "Liste les appareils Bluetooth LE\n"
     "a portee (5s d'ecoute).\n"},
};
