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
écrite.** Toutes les fonctions actives sont verrouillées par un geste
physique — maintenir le bouton **RETOUR** au moment de déclencher l'action
(voir "Verrou TX" plus bas) : sans ça, rien ne s'émet, quoi que dise
l'interface.

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
| Wi-Fi | Deauth ciblé (BSSID + MAC client au choix) | **oui — maintenir RETOUR** |
| Wi-Fi | Beacon spam (SSID que tu fournis, test WIDS/rogue-AP) | **oui — maintenir RETOUR** |
| Wi-Fi | Faux portail captif (page générique, logs locaux uniquement) | **oui — maintenir RETOUR** |
| BLE | Scan/inventaire des appareils BLE alentour | non |
| 2.4GHz | Scan d'activité par canal (NRF24, façon analyseur de spectre) | non |
| Sub-GHz | Scan RSSI par fréquence (CC1101) | non |
| Sub-GHz | Capture d'un signal (ex: ta propre télécommande de portail) | non |
| Sub-GHz | Rejeu d'une capture | **oui — maintenir RETOUR** |
| IR | Toggle marche/arrêt TV (codes courants, best-effort) | non* |
| IR | Apprentissage + rejeu de n'importe quel bouton de télécommande | non* |
| Wardriving | Log CSV horodaté + géolocalisé (Wi-Fi + BLE), téléchargeable | non |
| Interface | Panneau de contrôle web + menu 4 boutons sur l'écran TFT | - |

\* L'IR n'est pas verrouillé par le geste RETOUR : c'est une télécommande
universelle (portée optique de quelques mètres, aucune tierce personne
affectée au-delà de "l'écran s'éteint"), pas un outil réseau/RF à impact
large.

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
| TFT ILI9341 2.8" (SPI) | CS | 10 |
| TFT ILI9341 2.8" (SPI) | DC | 14 |
| TFT ILI9341 2.8" (SPI) | RST | 21 |
| TFT ILI9341 2.8" (SPI) | LED (rétroéclairage) | vers 3.3V (ou un GPIO libre type 47/48 si tu veux le piloter) |
| CC1101 | CS | 15 |
| CC1101 | GDO0 | 16 |
| CC1101 | GDO2 | 4 |
| NRF24L01 | CS | 5 |
| NRF24L01 | CE | 6 |
| NRF24L01 | IRQ (optionnel, non utilisé) | 7 |
| Récepteur IR (ex: TSOP38238/VS1838B) | OUT | 39 |
| LED IR (via transistor driver) | signal | 42 |
| Buzzer | signal | 38 |
| Bouton HAUT | signal → GND | 1 |
| Bouton BAS | signal → GND | 2 |
| Bouton SELECT | signal → GND | 40 |
| Bouton RETOUR | signal → GND | 41 |

Le buzzer, les boutons et l'électronique IR sont alimentés en 3.3V comme le
reste (l'ESP32-S3 ne tolère pas le 5V sur ses GPIO — si ton module IR est
en 5V only, passe par un level-shifter ou un transistor NPN comme étage de
sortie, ce qui isole aussi le niveau logique). Le TFT, CC1101 et NRF24L01
partagent le même bus SPI mais ont chacun leur propre CS — c'est géré par
le firmware. Les 4 boutons vont chacun d'un GPIO à la masse ; le firmware
active les pull-ups internes (`INPUT_PULLUP`), donc **pas besoin de
résistances externes**, juste un bouton-poussoir par ligne.

### L'interrupteur à glissière : alimentation, pas un GPIO

Le slide switch n'est **plus lu par le firmware** — il sert d'interrupteur
marche/arrêt de l'appareil, câblé **en série avec le +batterie**, avant le
régulateur/l'entrée VBAT. Position OFF = plus aucune alimentation, le
firmware n'a rien à gérer. Ça libère le GPIO39, réutilisé ici pour le
récepteur IR.

### Verrou TX : maintenir RETOUR, pas un interrupteur dédié

Sans switch matériel disponible pour ce rôle, le verrou des fonctions
actives (deauth, beacon spam, faux portail, rejeu sub-GHz) est maintenant
un geste physique : **il faut maintenir le bouton RETOUR appuyé au moment
exact où l'action se déclenche** (`TxArm::isArmed()` lit l'état du bouton
en direct, voir `src/tx_arm.cpp`). Ça marche pareil que tu déclenches
l'action depuis le menu à l'écran (maintiens RETOUR d'une main, appuie sur
SELECT de l'autre) ou depuis le panneau web (il faut alors que quelqu'un
soit physiquement devant l'appareil, doigt sur RETOUR, au moment où la
requête HTTP arrive). Relâcher RETOUR avant l'action = bloqué.

C'est une garantie plus faible qu'un interrupteur dédié qu'on arme à
l'avance, mais ça reste un geste physique délibéré et simultané, pas une
case à cocher logicielle. Si tu préfères récupérer un vrai verrou dédié,
ajoute un second interrupteur sur un GPIO libre (47 ou 48) et remplace le
contenu de `TxArm::isArmed()` par sa lecture.

## Build & flash

Le projet utilise [PlatformIO](https://platformio.org/).

```bash
pip install platformio
pio run                 # compile
pio run -t upload       # flashe (ESP32-S3 en mode USB-Serial/JTAG ou UART selon ton câblage)
pio device monitor       # logs série (115200 bauds)
```

La compilation a été vérifiée sur cet environnement (`pio run` → succès,
~24% de la flash 16MB utilisée, ~21.5% de la RAM).

Note écran : `tft.setRotation(1)` est utilisé pour l'affichage en paysage
320x240. Si l'image sort à l'envers ou dans le mauvais sens une fois câblé,
change juste ça en `setRotation(3)` dans `src/display.cpp`.

## Utilisation

### Via le panneau web (toujours actif)

1. Au démarrage, l'écran affiche le SSID du point d'accès de contrôle
   (`ESP32-Audit-XXXX`, mot de passe par défaut `auditctrl123` — **à changer**
   dans `include/config.h` avant usage réel).
2. Connecte un téléphone ou un PC à cet AP.
3. Ouvre `http://192.168.4.1:8080/` (port 8080, pas 80 — le port 80 est
   réservé à la page du faux portail captif quand il tourne).
4. Les sections deauth/beacon-spam/faux-portail/rejeu sub-GHz rappellent
   qu'il faut maintenir RETOUR sur l'appareil au moment de cliquer.

### Via le menu à 4 boutons

- Depuis l'écran d'accueil (horloge/GPS/état), **n'importe quel bouton**
  ouvre le menu principal.
- **HAUT/BAS** : déplacer la sélection. **SELECT** : valider. **RETOUR**
  (appui bref) : revenir en arrière.
- Pour une action marquée "maintenir RETOUR" (deauth, beacon spam, faux
  portail, rejeu sub-GHz) : maintiens RETOUR enfoncé puis appuie sur
  SELECT pendant que tu le maintiens — un appui bref sur RETOUR seul reste
  juste de la navigation.
- Le menu couvre : scan Wi-Fi (avec, par AP, deauth ciblé ou sniff des
  clients), scan BLE, scan 2.4GHz, scan/capture/rejeu sub-GHz, bascule
  beacon-spam et faux-portail (avec les SSID par défaut définis dans
  `config.h` — `DEFAULT_BEACON_SSIDS` / `DEFAULT_PORTAL_SSID`, à éditer
  puisque taper du texte libre avec 4 boutons n'est pas réaliste), IR
  (toggle TV / apprendre / rejouer), snapshot wardriving, et l'état du
  verrou TX.
- Le panneau web reste utilisable en parallèle pour tout ce qui demande de
  taper du texte (BSSID précis, SSID personnalisés, etc.).

L'écran affiche en continu, hors menu : heure RTC, état du fix GPS, nombre
de clients connectés à l'AP, état du verrou TX (maintenu ou non), et la
dernière action effectuée.

### IR (télécommande universelle)

- **Toggle TV power** : envoie une petite liste de codes NEC/Sony/RC5
  courants pour éteindre/allumer un écran — best-effort façon TV-B-Gone, ça
  ne marchera pas sur tous les modèles.
- **Apprendre** : capture le timing brut du prochain bouton pressé sur une
  vraie télécommande pointée vers le récepteur (5s d'écoute) — fiable, quel
  que soit le protocole.
- **Rejouer** : renvoie la dernière capture apprise.

## Structure du projet

```
include/        headers + config.h (pinout) + webui.h (page HTML embarquée)
src/            un module par domaine (rtc_clock, gps_module, display,
                buzzer, tx_arm, buttons, menu, wifi_tools, ble_tools,
                nrf24_tools, subghz, deauth, beacon_spam, evil_portal,
                ir_tools, wardriving, web_ctrl) + main.cpp
partitions_16mb.csv   table de partitions (app + LittleFS pour les logs)
```

## Pistes d'évolution

- Batterie + charge (le boîtier n'a actuellement pas de gestion d'énergie).
- Export des captures sub-GHz au format compatible Flipper Zero (`.sub`).
- Carte SD si les logs dépassent la capacité LittleFS restante (~9.6MB).
- Saisie de texte sur l'écran (BSSID/SSID) via les 4 boutons façon
  "T9"/liste défilante, pour ne plus dépendre uniquement du panneau web.
- Second interrupteur dédié au verrou TX si tu préfères ça au "maintenir
  RETOUR" (voir section verrou TX ci-dessus).
