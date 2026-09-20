# HARDWARE.md — Outil d'audit de sécurité portable (ESP32-S3)

> Spécification matérielle de référence du projet.
> Sert de source unique pour le câblage physique **et** la configuration firmware.
> Tout le firmware (PlatformIO / Arduino-ESP32) doit se conformer à ce brochage.

---

## 1. Carte principale

- **MCU** : ESP32-S3 **N16R8**
  - 16 Mo de flash (QIO)
  - **8 Mo de PSRAM OCTALE (OPI)** — point critique pour le brochage (voir §5)
  - Dual core + LP core, 240 MHz, Wi-Fi + BT 5 LE
- **Format** : type DevKitC, USB-UART **CP2102** (port de flash/console)
- **Port utilisé pour flash/monitor** : celui du CP2102 → `/dev/ttyUSB0` (l'USB natif du S3 n'est pas utilisé)

---

## 2. Liste des modules

| Module | Interface | Détail |
|---|---|---|
| Écran **TFT ILI9341 2.8"** 320×240 (non tactile) | SPI | lecteur microSD embarqué **non câblé** |
| **CC1101** sub-GHz — **433 MHz** | SPI | connecteur SMA, antenne fournie |
| **NRF24L01+PA/LNA** 2.4 GHz | SPI | connecteur SMA, antenne fournie |
| **GPS NEO-6M** | UART | 9600 baud par défaut |
| **RTC DS3231** | I2C | adresse `0x68` |
| **Buzzer** | GPIO | actif ou passif selon modèle |
| **4 boutons** | GPIO | `INPUT_PULLUP`, actifs bas |
| **IR émetteur** (LED 940 nm) + **récepteur** (VS1838B, 38 kHz) | GPIO | émetteur via transistor pour la portée |
| **Batterie LiPo 1000 mAh** | — | via chaîne d'alim (voir §4) |
| **TP4056 avec protection** (DW01 + 8205A) | — | chargeur USB, sorties `OUT+/OUT-` |
| **MT3608** (version pads) | — | boost réglé sur **5,0 V** |
| Interrupteur à glissière | — | coupe l'alim (pas de GPIO) |

> ⚠️ L'OLED SSD1306 1" initialement prévu est **abandonné** au profit du TFT.
> Ne pas le câbler (doublon de bus, écran trop petit).

---

## 3. Brochage complet (VERROUILLÉ)

| Bloc | Signal | GPIO |
|---|---|---|
| **I2C** (RTC DS3231) | SDA | **8** |
| | SCL | **9** |
| **SPI partagé** (TFT + CC1101 + NRF24) | SCK | **12** |
| | MOSI | **11** |
| | MISO | **13** |
| **CC1101** (433 MHz) | CSN | **10** |
| | GDO0 | **4** |
| | GDO2 | **40** |
| **NRF24L01+PA/LNA** | CSN | **14** |
| | CE | **15** |
| | IRQ | **41** |
| **TFT ILI9341** | CS | **5** |
| | DC | **16** |
| | BLK (rétroéclairage) | **48** |
| | RST | relié au **reset de la carte** (pas de GPIO) |
| **GPS NEO-6M** | RX_ESP (← TX GPS) | **18** |
| | TX_ESP (→ RX GPS) | **17** |
| **Buzzer** | I/O | **21** |
| **Boutons ×4** | Haut | **1** |
| | Bas | **2** |
| | OK | **6** |
| | Retour | **42** |
| **Batterie** | mesure ADC (via pont diviseur) | **7** |
| **IR** | émetteur (TX) | **38** |
| | récepteur (RX) | **39** |
| **Marge** | libre | **47** |

**Bus partagés :**
- **I2C** : un seul périphérique (DS3231, `0x68`). Bus libre pour extension.
- **SPI** : TFT + CC1101 + NRF24, **chacun son CS**. Les trois signaux SCK/MOSI/MISO sont communs.

**Notes de câblage :**
- Boutons : un fil par bouton vers **GND**, déclarés `INPUT_PULLUP` (pull-up interne, pas de résistance externe).
- Toutes les logiques sont en **3,3 V** (TFT, CC1101, NRF24, GPS, RTC) → pas de level-shifter.
- Les rôles des boutons sont purement logiciels → remappables librement.

---

## 4. Chaîne d'alimentation

```
USB ──► TP4056 (charge + protection) ──► interrupteur ──► MT3608 (boost 5 V) ──► carte ESP32-S3 (→ 3,3 V interne)
             │                                                                        │
          batterie LiPo 1S                                                    VIN / pin 5 V
```

- **TP4056** : alimentation tirée **exclusivement de `OUT+/OUT-`** (jamais `B+/B-`).
  Le module possède la protection (DW01 + 8205A) → coupe la décharge sous ~2,5 V, uniquement sur `OUT`.
- **Interrupteur** : entre `OUT+` du TP4056 et `VIN+` du MT3608.
- **MT3608** : boost réglé sur **5,0 V**, injecté dans l'entrée 5 V / VIN de la carte (le régulateur de la carte refait le 3,3 V).

---

## 5. Pièges à ne PAS rater

### Alimentation
- 🔴 **MT3608 : régler la sortie à 5,0 V AVANT de brancher la carte.** Le potentiomètre sort d'usine en position aléatoire (peut cracher 12–20 V → **grille la carte instantanément**). Procédure : alimenter l'entrée, sortie **débranchée**, mesurer au multimètre, tourner la vis jusqu'à **5,0 V**, **puis** brancher.
- **Condensateur boost** : 100–470 µF entre `VOUT+/VOUT-` du MT3608 (encaisse les pics Wi-Fi / NRF24).
- **Alim TP4056** : toujours `OUT+/OUT-`, jamais `B+/B-` (sinon la protection est court-circuitée).

### GPIO interdits sur ce N16R8
- 🔴 **PSRAM octale → GPIO 26 à 37 INTERDITS** (utilisés par flash + PSRAM). N'y toucher sous aucun prétexte.
- USB natif : 19/20 — évités.
- Console série (U0) : 43/44 — évités.
- Strapping : 0 / 3 / 45 / 46 — évités.
- ⚠️ **LED RGB embarquée** : selon la révision de carte, une WS2812 peut être câblée sur **GPIO 48** (parfois 38). On y a mis BLK (48) et IR TX (38). Vérifier la carte : en cas de conflit, permuter avec la broche libre 47.

### Radios
- 🔴 **NRF24+PA/LNA : condensateur de découplage 10 µF (voire 100 µF) entre VCC et GND, au plus près du module.** Sans ça : pics de courant → brownout → module « non détecté » ou resets aléatoires. Cause n°1 des galères NRF24.
- **CC1101 = 433 MHz** (marqué `433M` sur le PCB). Déclarer **433 MHz** comme fréquence de base dans le firmware (PAS 868).
- Antenne CC1101 (433) et antenne NRF24 (2,4 G) **non interchangeables** — déjà fournies avec les modules.

### Écran TFT
- **Contrôleur : ILI9341** (2.8" / 320×240).
- **Fils SPI vers l'écran = COURTS.** De longs dupont → artefacts d'affichage.
- **Partage du bus SPI** : le TFT (rapide) cohabite avec les radios (lentes). Gérer les vitesses **par transaction SPI**. TFT_eSPI utilise sa propre instance SPI → s'assurer qu'elle vise le même contrôleur SPI matériel que les radios et gérer le partage (begin/endTransaction). **Point d'intégration à valider tôt.**

---

## 6. Configuration PlatformIO (point de départ)

`platformio.ini` de base pour le N16R8 (PSRAM octale + flash 16 Mo) :

```ini
[env:esp32-s3-audit]
platform = espressif32@^6.9.0
board = esp32-s3-devkitc-1
framework = arduino
monitor_speed = 115200
upload_speed = 460800

; --- N16R8 : flash 16 Mo (QIO) + PSRAM 8 Mo OCTALE (OPI) ---
board_build.arduino.memory_type = qio_opi
board_upload.flash_size = 16MB
board_build.partitions = default_16MB.csv
build_flags =
    -DBOARD_HAS_PSRAM
    -DARDUINO_USB_CDC_ON_BOOT=0        ; port CP2102 (UART), pas l'USB natif

lib_deps =
    bodmer/TFT_eSPI                    ; écran ILI9341
    h2zero/NimBLE-Arduino              ; BLE (suite d'audit — voir FEATURES.md)
    crankyoldgit/IRremoteESP8266       ; IR émission/réception
    nrf24/RF24                         ; NRF24L01+
    jgromes/RadioLib                   ; CC1101 (433 MHz)
    mikalhart/TinyGPSPlus              ; parsing NMEA du NEO-6M
    adafruit/RTClib                    ; DS3231
```

**Config TFT_eSPI** (via `build_flags`, à ajuster — c'est l'étape la plus casse-pieds du TFT) :

```ini
    -DUSER_SETUP_LOADED
    -DILI9341_DRIVER
    -DTFT_MISO=13
    -DTFT_MOSI=11
    -DTFT_SCLK=12
    -DTFT_CS=5
    -DTFT_DC=16
    -DTFT_RST=-1          ; RST reliée au reset carte
    -DTFT_BL=48           ; rétroéclairage (BLK)
    -DLOAD_GLCD
    -DSPI_FREQUENCY=40000000
```

---

## 7. Ordre de montée en puissance (montage + code, étape par étape)

À chaque étape : compiler → flasher → vérifier avant de passer à la suivante. Ainsi, quand un truc casse, on sait exactement quel module l'a cassé.

1. **Squelette PlatformIO** + `platformio.ini` correct N16R8 (PSRAM OPI + partition 16 Mo). Vérifier que ça boote (log série).
2. **TFT seul** → afficher « hello ». Valide le SPI + l'écran + la config TFT_eSPI.
3. **+ DS3231** (I2C) → lire/afficher l'heure. Valide le bus I2C.
4. **+ boutons + mini UI de menu** (navigation Haut/Bas/OK/Retour).
5. **+ GPS** (parsing NMEA, affichage position/heure UTC).
6. **+ CC1101 puis NRF24** — le **SPI partagé en dernier**, c'est le plus délicat (cohabitation avec le TFT).
7. **+ IR** (émission puis réception, lib IRremoteESP8266).
8. **+ suite BLE** (voir `FEATURES.md`).

---

## 8. État des GPIO

Après ce brochage, la carte est **quasiment au maximum** des GPIO utilisables du S3 (le reste est mangé par flash/PSRAM/USB). Il reste **1 broche de marge (GPIO 47)** — réservée au CS du lecteur microSD du TFT ou à une LED d'état. Tout ajout supplémentaire nécessiterait un expandeur I2C ou de libérer une fonction.
