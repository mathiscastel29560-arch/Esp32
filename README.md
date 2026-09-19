# ESP32-S3 Audit Tool

Firmware pour un boîtier d'audit sécurité portable, inspiré de projets comme
Bruce/Flipper Zero, mais bâti autour d'un **ESP32-S3-N16R8** avec horloge
temps réel et GPS pour horodater et géolocaliser chaque relevé.

## ⚠️ Cadre d'utilisation

Cet outil embarque des fonctions de reconnaissance radio (Wi-Fi, BLE, 2.4GHz,
sub-GHz) et une fonction de rejeu de signal sub-GHz. **À n'utiliser que sur
tes propres équipements ou dans le cadre d'un audit pour lequel tu as une
autorisation écrite.** Le rejeu sub-GHz (CC1101) est protégé par un
interrupteur matériel (voir plus bas) : sans lui, l'appareil ne peut rien
émettre.

Volontairement, **ce firmware n'inclut pas** de deauth Wi-Fi, de flood de
faux beacons, ni de faux portail captif pour récupérer des identifiants.
Ces fonctions ciblent des tiers de façon large (n'importe quel client Wi-Fi
à portée, n'importe quel visiteur du faux réseau) plutôt qu'un équipement
précis que tu es autorisé à tester, donc je ne les ai pas construites. Tout
le reste (scan passif, écoute, sniffing, capture/rejeu RF sur un équipement
donné) est présent.

## Fonctions

| Domaine | Fonction | Actif (TX) ? |
|---|---|---|
| Horloge | DS3231, horodatage de tous les logs | non |
| GPS | Position temps réel, tag GPS des logs | non |
| Wi-Fi | Scan des réseaux (SSID/BSSID/RSSI/canal/chiffrement) | non |
| Wi-Fi | Observation passive des clients d'un AP donné | non |
| BLE | Scan/inventaire des appareils BLE alentour | non |
| 2.4GHz | Scan d'activité par canal (NRF24, façon analyseur de spectre) | non |
| Sub-GHz | Scan RSSI par fréquence (CC1101) | non |
| Sub-GHz | Capture d'un signal (ex: ta propre télécommande de portail) | non |
| Sub-GHz | Rejeu d'une capture | **oui — interrupteur requis** |
| Wardriving | Log CSV horodaté + géolocalisé (Wi-Fi + BLE), téléchargeable | non |
| Interface | Panneau de contrôle web (pas de boutons sur le boîtier) | - |

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

Le buzzer et l'interrupteur sont alimentés en 3.3V comme le reste (l'ESP32-S3
ne tolère pas le 5V sur ses GPIO). OLED, CC1101 et NRF24L01 partagent le même
bus SPI mais ont chacun leur propre CS — c'est géré par le firmware.

### L'interrupteur à glissière : sécurité physique, pas logicielle

`PIN_SAFETY_SWITCH` (GPIO39) est lu directement par le firmware : tant qu'il
est en position basse, **aucune fonction d'émission ne peut s'activer**,
quoi que dise l'interface web. C'est un choix volontaire — un simple
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
~22% de la flash 16MB utilisée, ~21% de la RAM).

## Utilisation

1. Au démarrage, l'écran OLED affiche le SSID du point d'accès de contrôle
   (`ESP32-Audit-XXXX`, mot de passe par défaut `auditctrl123` — **à changer**
   dans `include/config.h` avant usage réel).
2. Connecte un téléphone ou un PC à cet AP.
3. Ouvre `http://192.168.4.1/` : c'est le panneau de contrôle (pas de menu
   sur l'écran OLED, qui ne sert qu'à l'état — l'appareil n'a pas de
   boutons physiques pour naviguer un menu).
4. L'écran OLED affiche en continu : heure RTC, état du fix GPS, nombre de
   clients connectés à l'AP, état de l'interrupteur de sécurité, et la
   dernière action effectuée.

## Structure du projet

```
include/        headers + config.h (pinout) + webui.h (page HTML embarquée)
src/            un module par domaine (rtc_clock, gps_module, display,
                buzzer, safety_switch, wifi_tools, ble_tools, nrf24_tools,
                subghz, wardriving, web_ctrl) + main.cpp
partitions_16mb.csv   table de partitions (app + LittleFS pour les logs)
```

## Pistes d'évolution

- Écran plus grand ou boutons physiques pour un vrai menu autonome (sans
  passer par le Wi-Fi).
- Batterie + charge (le boîtier n'a actuellement pas de gestion d'énergie).
- Export des captures sub-GHz au format compatible Flipper Zero (`.sub`).
- Carte SD si les logs dépassent la capacité LittleFS restante (~9.6MB).
