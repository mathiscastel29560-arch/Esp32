# FEATURES.md — Toolkit Complet d'Audit Offensif ESP32-S3

> Spécification fonctionnelle complète de tous les modules d'audit et d'attaque.
> Voir `HARDWARE.md` pour la carte et le brochage.

---

## 🎯 Cadre d'usage (à lire avant d'implémenter)

Cet outil est un **instrument d'audit de sécurité professionnel**, destiné à des tests **ciblés et autorisés** :
- reconnaissance passive de l'environnement radio,
- audit d'appareils **que l'on possède ou que l'on est explicitement autorisé à tester**,
- détection défensive d'attaques,
- test de robustesse **de ses propres équipements**, en **environnement RF isolé**.

**TX ARM Protection:** Toute transmission RF requiert l'appui long du bouton BACK pour armer. Prévient les activations accidentelles.

---

## 📊 Architecture & Capacité

**Plateforme:** ESP32-S3-N16R8 (16 MB Flash, 8 MB PSRAM)
- **Flash utilisée:** 59.4%
- **Flash disponible:** 40.6%
- **RAM utilisée:** 24.6%
- **Radios embarquées:** CC1101 (Sub-GHz 433MHz), NRF24L01+ (2.4GHz), WiFi, BLE
- **Capteurs:** GPS (NEO-6M), IR TX/RX

---

## 🛠️ MODULE 1 — RECONNAISSANCE PASSIVE

### WiFi Tools
| ID | Fonction | Mode | TX ARM |
|---|---|---|---|
| 1.1 | WiFi Network Scan | Passif, détection APs | Non |
| 1.2 | Hidden Network Revealer | Révèle SSIDs cachés | Non |
| 1.3 | IoT Device Hunter | Pattern matching (Amazon, Google, TP-Link, etc) | Non |
| 1.4 | Smart Lock Scanner | Détecte serrures connectées | Non |
| 1.5 | Frequency Analyzer | Analyse bandes 2.4GHz + 433MHz | Non |

### BLE Tools
| ID | Fonction | Mode | TX ARM |
|---|---|---|---|
| 2.1 | BLE Device Scan (5s) | Détection passif, RSSI + UUID | Non |
| 2.2 | BLE Spam Watch (Detector) | Alerte spam beacons | Non (passif) |

### RF/2.4GHz Tools
| ID | Fonction | Mode | TX ARM |
|---|---|---|---|
| 3.1 | 2.4GHz Spectrum Scan | Détection activité/channel | Non |
| 3.2 | Drone Tracker (RSSI) | Détecte drones (freq hopping) | Non |
| 3.3 | Signal Sniffer (NRF24) | Capture paquets NRF24 | Non |
| 3.4 | Sub-GHz Scanner | 433.05-434.79 MHz + classification | Non |

---

## ⚔️ MODULE 2 — ATTAQUES OFFENSIVES

### WiFi Attacks
| ID | Attaque | Méthode | TX ARM | Notes |
|---|---|---|---|---|
| 4.1 | WiFi Deauth | IEEE 802.11 déauthentication frames | ✓ | Déconnecte clients (~3000 fps) |
| 4.2 | WiFi Jammer Suite | Channel jamming + Beacon DoS | ✓ | Hybrid 2 en 1 |
| 4.3 | Beacon Spam | Fake APs (FBI Van, etc) | ✓ | Crée APs fictifs |
| 4.4 | Evil Portal | Rogue AP captive portal | ✓ | Capture credentials via HTTP |
| 4.5 | Association Hijacker | MAC spoofing + takeover | ✓ | Prend place d'un client |
| 4.6 | HTTP Downgrade (SSL Strip) | HTTPS → HTTP interception | ✓ | Capture formulaires creds |
| 4.7 | WPA2 Handshake Cracker | Capture + dictionary attack | ✓ | 20 common passwords |

### Sub-GHz Attacks (433 MHz)
| ID | Attaque | Cible | TX ARM | Notes |
|---|---|---|---|---|
| 5.1 | Sub-GHz Bruteforce | Generic locks/garage/car/alarm | ✓ | 32 codes pré-loaded |
| 5.2 | Sub-GHz Replay | Capture → rejeu | ✓ | Interrupt-driven pulse capture |
| 5.3 | Sub-GHz Jammer Suite | Device reset + noise | ✓ | Noise generation 433MHz |
| 5.4 | Advanced RF Jammer | Noise/sweep/hopping follow | ✓ | 3 méthodes |
| 5.5 | Jamming Signal Generator | White/pink noise + sweep | ✓ | Pure jamming signal |

### BLE Attacks
| ID | Attaque | Méthode | TX ARM | Notes |
|---|---|---|---|---|
| 6.1 | BLE Address Spoof | MAC changer | ✓ | Simule autre device |
| 6.2 | BLE Pairing Attack | MITM pairing interception | ✓ | 5-step simulation |
| 6.3 | BLE DoS Attack | Link layer flooding | ✓ | Déconnecte appareils |
| 6.4 | BLE Beacon Spam | Continuity/FastPair/SwiftPair flood | ✓ | Random MAC, 500µs spacing |
| 6.5 | BLE Advertising Jammer | 3 modes: NOISE/FLOODING/SYNC | ✓ | Brouille advertisements |
| 6.6 | Bluetooth Aggressive Jammer | Hybrid BLE jamming | ✓ | Plus agressif |
| 6.7 | BLE Advanced Attack Suite | GATT jammer + eavesdropper | ✓ | Hybrid attack |

### NRF24 Attacks (2.4 GHz)
| ID | Attaque | Cible | TX ARM | Notes |
|---|---|---|---|---|
| 7.1 | NRF24 Replay Attack | Capture → rejeu | ✓ | Direct frame replay |
| 7.2 | NRF24 Packet Injection | Custom payload transmission | ✓ | Configurable data |

### IR Attacks
| ID | Attaque | Cible | TX ARM | Notes |
|---|---|---|---|---|
| 8.1 | IR TV Power Toggle | Télécommandes TV | ✓ | Power on/off |
| 8.2 | IR Bruteforce TV | Télécommandes TV | ✓ | Brute force 4500+ codes |
| 8.3 | IR Bruteforce AC | Climatiseurs | ✓ | Brute force commandes |
| 8.4 | IR Bruteforce Light | Ampoules intelligentes | ✓ | Brute force commandes |

### GPS Attacks
| ID | Attaque | Méthode | TX ARM | Notes |
|---|---|---|---|---|
| 9.1 | GPS Spoofing | NMEA generation + signal sim | ✓ | 3 modes: SIGNAL/GRADUAL/RANDOM |

---

## 🛡️ MODULE 3 — OUTILS DE SÉCURITÉ (Défensif)

| Outil | Fonction | Mode | Sortie |
|---|---|---|---|
| Battery Status | Charge + voltage | Passif | Display |
| GPS Map | Coordonnées actuelles | Passif | Display |
| TX Arm Status | Vérif du statut | Passif | Display |
| Dualboot OTA1 | Switch firmware partition | Manuel | Reboot |

---

## 📋 STATISTIQUES COMPLÈTES

### Par catégorie:
- **Reconnaissance:** 9 modules (passif)
- **WiFi Attacks:** 7 modules
- **Sub-GHz Attacks:** 5 modules
- **BLE Attacks:** 7 modules
- **NRF24 Attacks:** 2 modules
- **IR Attacks:** 4 modules
- **GPS Attacks:** 1 module
- **Défensif:** 4 modules
- **TOTAL:** 31 outils offensifs + 4 défensifs

### Utilisation ressources:
- Flash: 1.87 MB / 3.14 MB (59.4%)
- RAM: 80 KB / 320 KB (24.6%)
- Margin: 40.6% flash, 75.4% RAM

---

## 🎮 STRUCTURE MENU

### Menu WiFi (13 items)
```
0. Scan Networks
1. Reveal Hidden Networks
2. IoT Device Hunter
3. Smart Lock Scanner
4. Frequency Analyzer
5. Beacon Spam [START/STOP]
6. Evil Portal [START/STOP]
7. WiFi Deauth [START/STOP]
8. WiFi Jammer Suite [START/STOP]
9. Association Hijacker
10. HTTP Downgrade Attack
11. WPA2 Handshake Cracker
12. Back
```

### Menu BLE (10 items)
```
0. Scan Devices (5s)
1. BLE Address Spoof
2. BLE Pairing Attack
3. BLE DoS Attack
4. BLE Beacon Spam [START/STOP]
5. BLE Advertising Jam [START/STOP]
6. Bluetooth Aggressive Jam [START/STOP]
7. BLE Advanced Attacks [START/STOP]
8. BLE Spam Watch [START/STOP]
9. Check Spam Alert / Back
```

### Menu RF/2.4GHz (15 items)
```
0. 2.4GHz Spectrum Scan
1. Drone Tracker (RSSI)
2. Signal Sniffer (NRF24)
3. NRF24 Replay Attack
4. Sub-GHz Scanner
5. Sub-GHz Bruteforce
6. Sub-GHz Replay
7. IR: TV Power Toggle
8. IR: Bruteforce TV
9. IR: Bruteforce AC
10. IR: Bruteforce Light
11. GPS Spoofing (2.4GHz)
12. Advanced RF Jammer [START/STOP]
13. Sub-GHz Jammer Suite [START/STOP]
14. Jamming Signal Gen [START/STOP]
15. Back
```

---

## ⚙️ TECHNOLOGIES UTILISÉES

- **WiFi:** ESP32 native, esp_wifi_80211_tx() raw frames
- **BLE:** NimBLE-Arduino (lightweight)
- **Sub-GHz (433 MHz):** RadioLib + CC1101 direct modulation
- **NRF24:** RadioLib + direct GPIO control
- **IR:** IRremoteESP8266 library
- **GPS:** TinyGPSPlus + NEO-6M UART

---

## 🔒 SÉCURITÉ & PROTECTIONS

1. **TX ARM Protection:** Tous les transmetteurs requièrent l'appui du bouton BACK (> 500ms)
2. **Rate Limiting:** Délais entre paquets pour compliance FCC/régulations
3. **Isolation:** Modules indépendants, pas d'interactions croisées
4. **Logs:** Toutes opérations loggées sur Serial pour audit

---

## 📝 NOTES DE MAINTENANCE

- Micro-code de 31 modules (62 fichiers: headers + implémentations)
- Chaque module = namespace isolé
- Compilations testées: Flash 59.4%, RAM 24.6%
- Tous handlers menu intégrés et testés
- TX ARM enforced sur 25+ attaques

---

**Dernière mise à jour:** 2026-09-21
**Version:** 1.0 Offensive Toolkit Complete
