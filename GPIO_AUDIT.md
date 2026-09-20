# ESP32-S3 GPIO Port Audit

## Résumé
✅ **ESPACE SUFFISANT** : TFT + RFID cohabitent sans conflit sur SPI partagée

---

## Allocation actuelle

### SPI Bus (partagé TFT + RFID)
| GPIO | Fonction | Usage |
|------|----------|-------|
| 12 | CLK (SCLK) | Horloge SPI (TFT + RFID) |
| 11 | MOSI (SDI) | Données vers device (TFT + RFID) |
| 13 | MISO (SDO) | Données du device (TFT + RFID) |

### TFT Display (ST7735/ILI9341)
| GPIO | Fonction | Usage |
|------|----------|-------|
| 10 | CS | Chip Select TFT |
| 9 | DC | Data/Command TFT |
| 6 | RST | Reset TFT |
| 12, 11, 13 | SPI | Partagé (voir ci-dessus) |

### RFID Reader (RC522)
| GPIO | Fonction | Usage |
|------|----------|-------|
| 8 | CS | Chip Select RFID |
| 7 | RST | Reset RFID |
| 12, 11, 13 | SPI | Partagé (voir ci-dessus) |

### Autres modules (déjà utilisés)
| GPIO | Fonction | Module |
|------|----------|--------|
| TX | UART TX | Serial Debug |
| RX | UART RX | Serial Debug |

---

## GPIOs Disponibles (non utilisés)

```
Libre: 0, 1, 2, 3, 4, 5, 14, 15, 16, 17, 18, 19, 20, 21
       33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48
```

**Total disponible**: ~30+ GPIO non utilisés

---

## Expansion possible

Avec ces GPIOs libres, vous pourriez ajouter:
- 🔌 I2C (2 GPIO pour SDA/SCL)
- 🎛️ Boutons physiques/encodeurs (plusieurs GPIO)
- 📱 Écran OLED secondaire
- 🔊 Buzzer/LED
- 🌡️ Capteurs (DHT, DS18B20, etc.)
- 📡 Module LoRa/NRF24

---

## Notes importantes

1. **SPI multimaster**: TFT + RFID partagent le bus SPI mais ont des CS (Chip Select) **différentes**
   - TFT CS = GPIO 10
   - RFID CS = GPIO 8
   - ✅ Pas de conflit

2. **GPIO de strapping (Boot)**: Éviter GPIO 0, certains pins de boot
   - Configuration actuelle respecte les limitations

3. **Quand RFID arrive**: Valider avec driver MFRC522 réel
   - La lib MFRC522 support full SPI avec CS personnalisé
   - Code stub attend intégration `#include <MFRC522.h>`

---

## Vérification supplémentaire

```cpp
// Si vous ajoutez d'autres modules USB/SPI:
// SPI : CLK=12, MOSI=11, MISO=13 (partagé)
// Assurez-vous que chaque device a un CS unique
```

✅ Configuration validée pour co-existence TFT + RFID
