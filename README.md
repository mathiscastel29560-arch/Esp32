# ESP32-S3 Audit Tool

Firmware pour un boîtier d'audit sécurité portable, inspiré de projets comme
Bruce/Flipper Zero, mais bâti autour d'un **ESP32-S3-N16R8** avec horloge
temps réel et GPS pour horodater et géolocaliser chaque relevé. Pilotable
via un panneau web (Wi-Fi) et, si tu câbles 4 boutons, via un menu à l'écran.

## ⚠️ Cadre d'utilisation

Cet outil embarque des fonctions actives (deauth Wi-Fi, beacon spam, faux
portail captif, rejeu sub-GHz) en plus du scan/sniffing passif.
**À n'utiliser que sur tes propres équipements, dans un labo/CTF que tu
contrôles, ou dans le cadre d'un audit pour lequel tu as une autorisation
écrite.** Toutes les fonctions actives sont verrouillées par
l'interrupteur à glissière matériel (voir plus bas) : sans lui en position
armée, rien ne s'émet, quoi que dise l'interface.

**Volontairement absent : le BLE-spam** (flood de paquets d'appairage
factices type "Apple/Android proximity spam"). Contrairement au deauth
(cible un BSSID précis), au beacon spam (SSID que *tu* fournis) et au faux
portail (SSID que *tu* choisis), le BLE-spam n'a aucun mécanisme de
ciblage : il fait apparaître des popups sur *tous* les téléphones BLE à
portée, y compris ceux de tiers non impliqués. Le scan BLE passif reste
disponible pour la reconnaissance.

Le **beacon spam** et le **faux portail captif** émettent sur les ondes
publiques dès qu'ils tournent : même en labo perso, ils sortent des murs si
tu n'es pas en cage de Faraday. Garde ces tests dans un environnement où tu
sais qui est à portée.

## Fonctions

| Domaine | Fonction | Actif (TX) ? |
|---|---|---|
| Horloge | DS3231, horodatage de tous les logs | non |
| GPS | Position temps réel, tag GPS des logs | non |
| Wi-Fi | Scan des réseaux (SSID/BSSID/RSSI/canal/chiffrement) | non |
| Wi-Fi | Observation passive des clients d'un AP donné | non |
| Wi-Fi | Deauth ciblé (BSSID + MAC client au choix) | **oui — interrupteur requis** |
| Wi-Fi | Beacon spam (SSID que tu fournis, test WIDS/rogue-AP) | **oui — interrupteur requis** |
| Wi-Fi | Faux portail captif (page générique, logs locaux uniquement) | **oui — interrupteur requis** |
| BLE | Scan/inventaire des appareils BLE alentour | non |
| 2.4GHz | Scan d'activité par canal (NRF24, façon analyseur de spectre) | non |
| Sub-GHz | Scan RSSI par fréquence (CC1101) | non |
| Sub-GHz | Capture d'un signal (ex: ta propre télécommande de portail) | non |
| Sub-GHz | Rejeu d'une capture | **oui — interrupteur requis** |
| Wardriving | Log CSV horodaté + géolocalisé (Wi-Fi + BLE), téléchargeable | non |
| Interface | Panneau de contrôle web + menu 4 boutons optionnel sur l'écran | - |

## Câblage (ESP32-S3-N16R8)

Les GPIO 0, 3, 19, 20, 26-37, 43-46 sont évités volontairement (pins de
strapping, USB natif D+/D-, ou réservés à la PSRAM octale / flash quad du
module N16R8).

| Module | Signal | GPIO |
|---|---|---|
| DS3231 (I2C) | SDA | 8 |
| DS3231 (I2C) | SCL | 9 |
| GPS NEO-6M (UART1) | RX (← GPS TX) | 17 |
| GPS NEO-6M (UART1) | TX (→ GPS RX) | 18 |
| Bus SPI partagé | SCK | 12 |
| Bus SPI partagé | MISO | 13 |
| Bus SPI partagé | MOSI | 11 |
| OLED SSD1306 (SPI) | CS | 10 |
| OLED SSD1306 (SPI) | DC | 14 |
| OLED SSD1306 (SPI) | RST | 21 |
| CC1101 | CS | 15 |
| CC1101 | GDO0 | 16 |
| CC1101 | GDO2 | 4 |
| NRF24L01 | CS | 5 |
| NRF24L01 | CE | 6 |
| NRF24L01 | IRQ (optionnel, non utilisé) | 7 |
| Buzzer | signal | 38 |
| Interrupteur à glissière (sécurité TX) | signal | 39 |
| Bouton HAUT (optionnel) | signal → GND | 1 |
| Bouton BAS (optionnel) | signal → GND | 2 |
| Bouton SELECT (optionnel) | signal → GND | 40 |
| Bouton RETOUR (optionnel) | signal → GND | 41 |

Le buzzer, l'interrupteur et les boutons sont alimentés en 3.3V comme le
reste (l'ESP32-S3 ne tolère pas le 5V sur ses GPIO). OLED, CC1101 et
NRF24L01 partagent le même bus SPI mais ont chacun leur propre CS — c'est
géré par le firmware. Les 4 boutons vont chacun d'un GPIO à la masse ; le
firmware active les pull-ups internes (`INPUT_PULLUP`), donc **pas besoin de
résistances externes**, juste un bouton-poussoir par ligne.

### L'interrupteur à glissière : sécurité physique, pas logicielle

`PIN_SAFETY_SWITCH` (GPIO39) est lu directement par le firmware : tant qu'il
est en position basse, **aucune fonction d'émission ne peut s'activer**
(deauth, beacon spam, faux portail, rejeu sub-GHz), quoi que dise
l'interface web ou le menu à boutons. C'est un choix volontaire — un simple
interrupteur matériel donne une garantie qu'une case à cocher logicielle ne
donne pas.

## Build & flash

Le projet utilise [PlatformIO](https://platformio.org/).

```bash
pip install platformio
pio run                 # compile
pio run -t upload       # flashe (ESP32-S3 en mode USB-Serial/JTAG ou UART selon ton câblage)
pio device monitor       # logs série (115200 bauds)
```

La compilation a été vérifiée sur cet environnement (`pio run` → succès,
~22.5% de la flash 16MB utilisée, ~21.5% de la RAM).

## Utilisation

### Via le panneau web (toujours actif)

1. Au démarrage, l'écran OLED affiche le SSID du point d'accès de contrôle
   (`ESP32-Audit-XXXX`, mot de passe par défaut `auditctrl123` — **à changer**
   dans `include/config.h` avant usage réel).
2. Connecte un téléphone ou un PC à cet AP.
3. Ouvre `http://192.168.4.1:8080/` (port 8080, pas 80 — le port 80 est
   réservé à la page du faux portail captif quand il tourne).
4. Les sections deauth/beacon-spam/faux-portail sont marquées ⚠️ dans
   l'interface ; elles renvoient un échec explicite si l'interrupteur de
   sécurité n'est pas armé.

### Via le menu à 4 boutons (si câblés)

- Depuis l'écran d'accueil (horloge/GPS/état), **n'importe quel bouton**
  ouvre le menu principal.
- **HAUT/BAS** : déplacer la sélection. **SELECT** : valider. **RETOUR** :
  revenir en arrière (ou quitter le menu depuis l'accueil).
- Le menu couvre : scan Wi-Fi (avec, par AP, deauth ciblé ou sniff des
  clients), scan BLE, scan 2.4GHz, scan/capture/rejeu sub-GHz, bascule
  beacon-spam et faux-portail (avec les SSID par défaut définis dans
  `config.h` — `DEFAULT_BEACON_SSIDS` / `DEFAULT_PORTAL_SSID`, à éditer
  puisque taper du texte libre avec 4 boutons n'est pas réaliste),
  snapshot wardriving, et l'état de l'interrupteur de sécurité.
- Le panneau web reste utilisable en parallèle pour tout ce qui demande de
  taper du texte (BSSID précis, SSID personnalisés, etc.).

L'écran OLED affiche en continu, hors menu : heure RTC, état du fix GPS,
nombre de clients connectés à l'AP, état de l'interrupteur de sécurité, et
la dernière action effectuée.

## Structure du projet

```
include/        headers + config.h (pinout) + webui.h (page HTML embarquée)
src/            un module par domaine (rtc_clock, gps_module, display,
                buzzer, safety_switch, buttons, menu, wifi_tools, ble_tools,
                nrf24_tools, subghz, deauth, beacon_spam, evil_portal,
                wardriving, web_ctrl) + main.cpp
partitions_16mb.csv   table de partitions (app + LittleFS pour les logs)
```

## Pistes d'évolution

- Écran plus grand pour afficher plus de lignes dans le menu à boutons.
- Batterie + charge (le boîtier n'a actuellement pas de gestion d'énergie).
- Export des captures sub-GHz au format compatible Flipper Zero (`.sub`).
- Carte SD si les logs dépassent la capacité LittleFS restante (~9.6MB).
- Saisie de texte sur l'écran (BSSID/SSID) via les 4 boutons façon
  "T9"/liste défilante, pour ne plus dépendre uniquement du panneau web.
