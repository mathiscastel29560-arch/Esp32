# ESP32-S3 Audit Tool

Firmware pour un boîtier d'audit sécurité portable, inspiré de projets comme
Bruce/Flipper Zero, bâti autour d'un **ESP32-S3-N16R8**. Horloge DS3231 et
GPS NEO-6M pour horodater/géolocaliser chaque relevé, écran TFT ILI9341
2.8" + 4 boutons pour un usage autonome, et un panneau de contrôle web en
parallèle.

**Références matérielles/fonctionnelles :**
- [`HARDWARE.md`](./HARDWARE.md) — brochage verrouillé, chaîne d'alimentation, pièges de montage.
- [`FEATURES.md`](./FEATURES.md) — spec détaillée de la suite d'audit BLE (modules 1 à 4).

## ⚠️ Cadre d'utilisation

Cet outil embarque des fonctions actives (deauth Wi-Fi, beacon spam, faux
portail captif, rejeu sub-GHz, fuzzing BLE) en plus du scan/sniffing passif.
**À n'utiliser que sur tes propres équipements, dans un labo/CTF que tu
contrôles, ou dans le cadre d'un audit pour lequel tu as une autorisation
écrite.** Toutes les fonctions RF actives sont verrouillées par un geste
physique — maintenir le bouton **RETOUR** au moment de déclencher l'action
(voir "Verrou TX" plus bas) : sans ça, rien ne s'émet, quoi que dise
l'interface.

**Volontairement absent : le BLE-spam** (flood de paquets d'appairage
factices type "Apple/Android proximity spam"). Contrairement au deauth
(cible un BSSID précis), au beacon spam (SSID que *tu* fournis), au faux
portail (SSID que *tu* choisis) et au fuzzing BLE (une seule adresse
connectée), le BLE-spam n'a aucun mécanisme de ciblage : il fait apparaître
des popups sur *tous* les téléphones BLE à portée, y compris ceux de tiers
non impliqués. Le scan et la détection BLE passifs restent disponibles.

Le **beacon spam** et le **faux portail captif** émettent sur les ondes
publiques dès qu'ils tournent : même en labo perso, ils sortent des murs si
tu n'es pas en cage de Faraday. Le **fuzzing BLE (module 4)** doit être
réservé à un environnement RF isolé sur un appareil que tu possèdes — voir
`FEATURES.md`.

## Fonctions

| Domaine | Fonction | Actif (TX) ? |
|---|---|---|
| Horloge | DS3231, horodatage de tous les logs | non |
| GPS | Position temps réel, tag GPS des logs | non |
| Batterie | Tension/pourcentage LiPo via pont diviseur (ADC) | non |
| Wi-Fi | Scan des réseaux (SSID/BSSID/RSSI/canal/chiffrement) | non |
| Wi-Fi | Observation passive des clients d'un AP donné | non |
| Wi-Fi | Deauth ciblé (BSSID + MAC client au choix) | **oui — maintenir RETOUR** |
| Wi-Fi | Beacon spam (SSID que tu fournis, test WIDS/rogue-AP) | **oui — maintenir RETOUR** |
| Wi-Fi | Faux portail captif (page générique, logs locaux uniquement) | **oui — maintenir RETOUR** |
| BLE (M1) | Scan/inventaire : adresse, RSSI, nom, UUID services, fabricant | non |
| BLE (M2) | Audit GATT d'un appareil (lecture sans appairage, écriture sans auth, Just Works, fuites Device Info) | non* |
| BLE (M3) | Détecteur de BLE-spam (Continuity/Fast Pair/Swift Pair), défensif | non |
| BLE (M4) | Fuzzing GATT d'un seul appareil (écritures surdimensionnées/read-only, cycles reconnexion) | non* |
| 2.4GHz | Scan d'activité par canal (NRF24+PA/LNA, façon analyseur de spectre) | non |
| Sub-GHz | Scan RSSI par fréquence (CC1101, 433MHz) | non |
| Sub-GHz | Capture d'un signal (ex: ta propre télécommande de portail) | non |
| Sub-GHz | Rejeu d'une capture | **oui — maintenir RETOUR** |
| IR | Toggle marche/arrêt TV (codes courants, best-effort) | non** |
| IR | Apprentissage + rejeu de n'importe quel bouton de télécommande | non** |
| Wardriving | Log CSV horodaté + géolocalisé (Wi-Fi + BLE), téléchargeable | non |
| Interface | Panneau de contrôle web + menu 4 boutons sur l'écran TFT | - |

\* Modules BLE 2 et 4 : pas de verrou RETOUR, parce que ce sont des actions
**connectées à une seule adresse que tu donnes explicitement** — contrairement
au deauth/beacon-spam qui agissent sur les ondes en diffusion, il n'y a
personne d'autre que cet appareil-là qui puisse être affecté. Le module 4
reste à réserver à un environnement isolé (voir `FEATURES.md`).

\** L'IR n'est pas verrouillé non plus : télécommande universelle, portée
optique de quelques mètres, aucune tierce personne affectée au-delà de
"l'écran s'éteint".

## Câblage, alimentation, pièges de montage

Tout est dans [`HARDWARE.md`](./HARDWARE.md) — table de brochage complète
(verrouillée), chaîne d'alimentation (TP4056 → interrupteur → MT3608 boost
5V → carte), et la liste des pièges à ne pas rater (réglage du MT3608
**avant** de brancher la carte, condensateur de découplage NRF24,
fréquence CC1101 = 433MHz, etc.).

### Verrou TX : maintenir RETOUR, pas un interrupteur dédié

Le slide switch est l'alimentation générale de l'appareil (câblé côté
batterie, voir `HARDWARE.md` §4) — le firmware ne le lit pas. Le verrou des
fonctions RF actives (deauth, beacon spam, faux portail, rejeu sub-GHz) est
donc un geste physique : **maintenir le bouton RETOUR au moment exact où
l'action se déclenche** (`TxArm::isArmed()` lit l'état du bouton en direct,
voir `src/tx_arm.cpp`). Ça marche pareil que tu déclenches l'action depuis
le menu à l'écran (maintiens RETOUR d'une main, appuie sur OK de l'autre)
ou depuis le panneau web (il faut alors que quelqu'un soit physiquement
devant l'appareil, doigt sur RETOUR, au moment où la requête HTTP arrive).
Relâcher RETOUR avant l'action = bloqué. GPIO47 reste libre si tu préfères
un second interrupteur dédié à la place de ce geste.

## Build & flash

Le projet utilise [PlatformIO](https://platformio.org/).

```bash
pip install platformio
pio run                 # compile
pio run -t upload       # flashe (via le CP2102, /dev/ttyUSB0 typiquement)
pio device monitor       # logs série (115200 bauds)
```

La compilation a été vérifiée sur cet environnement (`pio run` → succès,
~42% des 3MB réservés à `ota_0` utilisés, ~17.7% de la RAM). Depuis le
passage à la table de partitions dual-boot (voir "Dual-boot avec ESP32-DIV"
plus bas), l'app n'a plus toute la flash 16MB pour elle — juste les 3MB
d'`ota_0` — d'où le pourcentage plus élevé qu'avant malgré une taille de
binaire inchangée.

Note écran : `tft.setRotation(Theme::ROTATION)` (`include/ui/theme.h`) pilote
l'orientation du TFT. Si l'image sort à l'envers, change cette constante.

### Écran : TFT (actuel) ou OLED (ancien) — détection automatique

Le firmware marche avec les deux écrans envisagés, sans rien à choisir à la
compilation : au boot, `Display::begin()` (`src/display.cpp`) teste dans
l'ordre —

1. **I2C** : sonde l'adresse `OLED_I2C_ADDR` (`0x3C` par défaut,
   `include/config.h`) sur le bus SDA=8/SCL=9. Si un SSD1306 répond, c'est
   lui qui est utilisé — le TFT/SPI n'est même pas touché.
2. **SPI** : sinon, initialise le TFT puis lit son registre d'auto-ID
   (`RDID4`, motif documenté par TFT_eSPI lui-même) pour confirmer qu'un
   vrai ILI9341 est branché — pas juste que l'init n'a pas planté. Si la
   valeur attendue (`0x93`) ne revient pas (bus flottant, rien de branché),
   bascule sur le mode headless.
3. **Aucun écran détecté** : mode headless, tout continue de tourner
   normalement (Wi-Fi/BLE/scans/logs), juste sans affichage local — le
   panneau web reste utilisable.

C'est aussi la vraie correction du crash au boot sans écran signalé plus tôt
(`StoreProhibited` dans `begin_tft_write()`) : la cause était un double
`SPI.begin()` sur le même objet `SPI` global (une fois dans `main.cpp`, une
fois à l'intérieur de `tft.init()`) — `main.cpp` ne l'appelle plus lui-même,
`Display::begin()` s'en charge une seule fois, dans le bon ordre selon
l'écran trouvé. **Pas testé sur le matériel réel par manque d'accès à la
carte pendant cette session** — à vérifier au prochain flash.

L'OLED SSD1306 obtient une UI simplifiée mais complète (même menu,
mêmes listes/écrans de détail, même confirmation TX) via
`src/ui/oled_ui.cpp` : texte monochrome sans animations ni sprites, pensé
pour un module 128x64 (`OLED_WIDTH`/`OLED_HEIGHT` dans `config.h` — passe à
32 si le tien est un 128x32).

## Utilisation

### Via le panneau web (toujours actif)

1. Au démarrage, l'écran affiche le SSID du point d'accès de contrôle
   (`ESP32-Audit-XXXX`, mot de passe par défaut `auditctrl123` — **à changer**
   dans `include/config.h` avant usage réel).
2. Connecte un téléphone ou un PC à cet AP.
3. Ouvre `http://192.168.4.1:8080/` (port 8080, pas 80 — le port 80 est
   réservé à la page du faux portail captif quand il tourne).
4. Les sections marquées ⚠️ rappellent qu'il faut maintenir RETOUR sur
   l'appareil au moment de cliquer.

### Via le menu à 4 boutons

- Depuis l'écran d'accueil (horloge/GPS/batterie/état), **n'importe quel
  bouton** ouvre le menu principal.
- **HAUT/BAS** : déplacer la sélection. **OK** : valider. **RETOUR**
  (appui bref) : revenir en arrière.
- Pour une action marquée "maintenir RETOUR" : maintiens RETOUR enfoncé
  puis appuie sur OK pendant que tu le maintiens — un appui bref sur
  RETOUR seul reste juste de la navigation.
- Le menu couvre : scan Wi-Fi (avec, par AP, deauth ciblé ou sniff des
  clients), scan BLE (avec, par appareil, audit GATT ou fuzz), scan
  2.4GHz, scan/capture/rejeu sub-GHz, bascule beacon-spam et faux-portail
  (SSID par défaut dans `config.h` — `DEFAULT_BEACON_SSIDS` /
  `DEFAULT_PORTAL_SSID`, à éditer puisque taper du texte libre avec 4
  boutons n'est pas réaliste), IR (toggle TV / apprendre / rejouer),
  surveillance BLE-spam (démarrer/arrêter/vérifier), snapshot wardriving,
  l'état du verrou TX, une carte GPS (contour simplifié de la France avec
  ta position en direct, `include/france_outline.h`), et une aide
  intégrée (un sujet par fonction : ce que ça fait, comment s'en servir,
  comment lire le résultat).
- Le panneau web reste utilisable en parallèle pour tout ce qui demande de
  taper du texte (BSSID/adresse BLE précise, SSID personnalisés, etc.).

L'écran affiche en continu, hors menu : heure RTC, état du fix GPS, niveau
de batterie, nombre de clients connectés à l'AP, état du verrou TX, et la
dernière action effectuée.

### Suite d'audit BLE (voir FEATURES.md)

- **Module 1 (scan)** : liste triée par RSSI, avec fabricant déduit du
  manufacturer data et UUID de services annoncés.
- **Module 2 (audit GATT)** : sélectionne un appareil depuis le scan,
  connexion cliente, énumération services/caractéristiques, et rapport :
  combien sont lisibles sans appairage, écrivibles sans authentification,
  si le pairing est "Just Works" (pas de protection MITM), et ce que le
  Device Information Service expose (numéro de série, version firmware…).
- **Module 3 (détection spam)** : surveillance passive continue,
  déclenche une alerte si trop de MAC aléatoires distinctes annoncent des
  beacons d'appairage (Continuity/Fast Pair/Swift Pair) en peu de temps —
  ça te dit juste que quelqu'un fait ça autour de toi, ça n'émet rien.
- **Module 4 (fuzzing)** : sur l'appareil sélectionné, écritures
  surdimensionnées, écritures sur des caractéristiques read-only, cycles
  connexion/déconnexion rapides — pour vérifier que la pile BLE de ton
  appareil tient le choc. **En isolation, sur du matériel à toi.**

### IR (télécommande universelle)

- **Toggle TV power** : envoie une petite liste de codes NEC/Sony/RC5
  courants — best-effort façon TV-B-Gone, ça ne marchera pas sur tous les
  modèles.
- **Apprendre** : capture le timing brut du prochain bouton pressé sur une
  vraie télécommande (5s d'écoute) — fiable, quel que soit le protocole.
- **Rejouer** : renvoie la dernière capture apprise.

## Dual-boot avec ESP32-DIV

Ce firmware peut cohabiter sur la même puce avec un second firmware
autonome, grâce au mécanisme de rollback OTA d'ESP-IDF. Le slot `ota_1`
héberge actuellement [ESP32-DIV](https://github.com/cifertech/esp32-div)
(binaire précompilé officiel, non modifié) :

- `partitions_16mb.csv` réserve deux slots d'application, `ota_0` (ce
  firmware) et `ota_1` (ESP32-DIV), plus `auditfs` (LittleFS de ce
  firmware — logs, wardriving, captures sub-GHz) et `spiffs` (système de
  fichiers propre à ce qui tourne sur `ota_1`). Les deux firmwares ne
  doivent jamais monter la même partition de données.
- Le bootloader ESP-IDF utilisé par Arduino-ESP32 a le rollback d'app
  activé par défaut (`CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE=y`). Ce
  firmware confirme chaque boot réussi via `DualBoot::markValid()`
  (`src/dualboot.cpp`, appelée en toute fin de `setup()`).
- Depuis le menu, "Boot ESP32-DIV" bascule sur `ota_1` et redémarre.
  **Important** : contrairement à Bruce (voir plus bas), ESP32-DIV est
  flashé ici depuis son binaire précompilé d'origine, sans le correctif
  `verifyRollbackLater()` décrit ci-dessous — son build confirme donc son
  propre boot comme valide dès `initArduino()`, avant que son code n'ait
  la moindre chance de planter. Le filet de sécurité "un cycle
  d'alimentation suffit à revenir sur ce firmware" **ne s'applique donc
  pas** à ce binaire : en cas de plantage/blocage sur ESP32-DIV, récupère
  manuellement en effaçant la partition `otadata` :
  ```bash
  esptool.py --chip esp32s3 erase_region 0xE000 0x2000
  ```
  (efface seulement les 2KB de sélection OTA — le bootloader retombe sur
  `ota_0` au boot suivant ; ne touche ni au bootloader, ni à la table de
  partitions, ni à `ota_0`/`ota_1` eux-mêmes.)

### Mise en place (une fois)

1. Flashe ce firmware normalement (`pio run -t upload` depuis la racine du
   projet) — ça écrit le bootloader, la table de partitions et `ota_0`.
2. Récupère et flashe le binaire précompilé ESP32-DIV (variante `v2`,
   pour ESP32-S3) directement sur `ota_1`, sans rien construire :

   ```bash
   git clone --depth 1 https://github.com/cifertech/esp32-div
   python3 -m esptool --chip esp32s3 --baud 460800 write-flash 0x310000 \
     "esp32-div/Pre-compiled Bin/ESP32-DIV-v2-v1.7.2.bin"
   ```

   Ça écrit uniquement l'image applicative à l'offset `0x310000` (`ota_1`)
   — ça ne touche jamais au bootloader, à la table de partitions ni à
   `ota_0`. Testé sur ce matériel : démarre sans planter.

### Limites connues

- ESP32-DIV attend un écran tactile TFT (SPI, piloté par TFT_eSPI) comme
  interface principale — plus de 2800 appels de dessin directs répartis
  dans tout son code, aucun mode texte/OLED. Tant que le TFT n'est pas
  câblé, l'écran OLED de ce projet reste figé sur sa dernière image (il ne
  reçoit plus rien une fois qu'on a basculé sur `ota_1`) : le firmware
  démarre bien, mais rien n'est visible sans le TFT.
- Ses boutons physiques passent par un expandeur I2C PCF8574 optionnel
  (le tactile XPT2046 est l'entrée principale) — pas les mêmes broches que
  nos boutons GPIO directs ; non câblé pour l'instant.
- Comme mentionné plus haut, pas de filet de sécurité rollback tant que ce
  binaire reste un build d'origine, non modifié.

### Alternative : Bruce (testé, écarté)

Le profil de carte pour dual-booter avec [Bruce](https://github.com/pr3y/Bruce)
existe toujours dans [`bruce-board/`](./bruce-board/) et reste utilisable
(brochage exact de `HARDWARE.md`, correctif `verifyRollbackLater()` inclus
dans `bruce-board/esp32-audit-dualboot/interface.cpp` pour que le filet de
sécurité fonctionne réellement avec Bruce — piège découvert en testant sur
du vrai matériel : sans ce correctif, `initArduino()` confirme le boot
avant même que Bruce ait pu planter). Écarté comme choix par défaut après
un crash reproductible et non corrigible depuis l'extérieur : panique
`StoreProhibited` très tôt au démarrage, dans le driver Wi-Fi propriétaire
et fermé d'Espressif (`libnet80211.a`) — confirmé identique sur la branche
`dev` de Bruce et sur son tag stable `v1.16.1`.

```bash
git clone https://github.com/pr3y/Bruce.git
./bruce-board/flash_bruce.sh /path/to/Bruce [/dev/ttyUSB0]
```

Pas de RTC ni de GPS côté Bruce (aucun driver DS3231 ; le NEO-6M est câblé
mais inutilisé sauf ajout d'un module GPS côté Bruce). Pas de carte SD
câblée sur cette carte (`SDCARD_CS=-1` dans le profil) — Bruce utilise son
`spiffs` (~6.4MB) pour scripts/captures.

## Structure du projet

```
HARDWARE.md      brochage verrouillé, alimentation, pièges de montage
FEATURES.md      spec de la suite d'audit BLE (modules 1-4)
include/         headers + config.h (pinout) + webui.h (page HTML embarquée)
src/             un module par domaine :
                 rtc_clock, gps_module, battery, display, buzzer, tx_arm,
                 buttons, menu, wifi_tools, deauth, beacon_spam,
                 evil_portal, ble_tools, ble_gatt_audit, ble_spam_detector,
                 ble_fuzzer, nrf24_tools, subghz, ir_tools, wardriving,
                 web_ctrl, mascot, dualboot, main.cpp
                 (+ src/ui/: ui, oled_ui, widgets — écran TFT et/ou OLED)
partitions_16mb.csv   table de partitions dual-boot (ota_0/ota_1 + auditfs/spiffs)
bruce-board/     profil de carte Bruce (alternative, écartée) + script de flash — voir "Dual-boot avec ESP32-DIV"
```

## Pistes d'évolution

- Export des captures sub-GHz au format compatible Flipper Zero (`.sub`).
- Carte SD si les logs dépassent la capacité LittleFS restante (~9.6MB) —
  le lecteur microSD du TFT n'est pas câblé pour l'instant (voir
  `HARDWARE.md` §2).
- Saisie de texte sur l'écran (BSSID/SSID/adresse BLE) via les 4 boutons
  façon "T9"/liste défilante, pour ne plus dépendre uniquement du panneau
  web pour les valeurs libres.
- Second interrupteur dédié au verrou TX si tu préfères ça au "maintenir
  RETOUR" (GPIO47 libre).
