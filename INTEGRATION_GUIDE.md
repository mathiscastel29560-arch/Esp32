# Guide d'Intégration Phase 1 : Ajouter une Nouvelle Attaque

## 📖 Vue d'ensemble

Ce guide explique comment ajouter une nouvelle attaque au système Phase 1 en utilisant les vrais drivers matériel. Le processus est simple grâce à l'architecture standardisée de Phase 0.

---

## 🏗️ Architecture de Base

### Hiérarchie des Classes

```
Attack (classe abstraite)
├── WiFiAttack (attaques WiFi)
├── BLEAttack (attaques Bluetooth)
├── RFAttack (attaques radio)
├── NFCAttack (attaques NFC)
├── DoSAttack (attaques déni de service)
└── AdvancedAttack (attaques avancées)
```

### Interface Minimale

Chaque attaque implémente ces 4 méthodes virtuelles :

```cpp
class MyNewAttack : public Attack {
public:
  void begin() override;    // Initialiser ressources
  void start() override;    // Démarrer l'attaque
  void update() override;   // Boucle principale (appelée tous les 50ms)
  void stop() override;     // Nettoyer ressources
};
```

### État et Cycle de Vie

```
IDLE → begin() → SCANNING → start() → ATTACKING → update() → SUCCESS/FAILED → stop()
```

---

## 📋 Étapes pour Ajouter une Attaque

### Étape 1 : Créer le Fichier Header

Créer `include/my_new_attack.h` :

```cpp
#ifndef MY_NEW_ATTACK_H
#define MY_NEW_ATTACK_H

#include "attack_framework.h"

class MyNewAttack : public Attack {
public:
  MyNewAttack();
  ~MyNewAttack();
  
  // Obligatoires
  void begin() override;
  void start() override;
  void update() override;
  void stop() override;
  
private:
  // État interne
  uint32_t startTime;
  uint16_t discoveredItems;
  
  // Drivers matériel (Phase 1)
  // HardwareDriver* driver;
};

#endif // MY_NEW_ATTACK_H
```

### Étape 2 : Implémenter l'Attaque

Créer `src/my_new_attack.cpp` :

```cpp
#include "my_new_attack.h"
#include "logging_system.h"
#include "results_builder.h"

MyNewAttack::MyNewAttack() 
  : Attack("Mon Attaque", "Description courte"),
    startTime(0),
    discoveredItems(0) {
  setCategory("WiFi");  // ou BLE, RF, NFC, DoS, Advanced
  
  // Configuration par défaut
  setParameter("timeout", "15000");  // 15 secondes
  setParameter("maxResults", "20");
}

MyNewAttack::~MyNewAttack() {
  // Libérer ressources si nécessaire
}

void MyNewAttack::begin() {
  Logger::getInstance().info(getName(), "Initialisation");
  
  // Phase 0: Simulation
  // Phase 1: Initialiser drivers matériel
  // driver->init();
  
  setStatus(AttackStatus::SCANNING);
}

void MyNewAttack::start() {
  startTime = millis();
  Logger::getInstance().info(getName(), "Démarrage");
  
  setStatus(AttackStatus::ATTACKING);
  isRunning = true;
}

void MyNewAttack::update() {
  if (!isRunning) return;
  
  uint32_t elapsed = millis() - startTime;
  uint32_t timeout = std::stoul(getParameter("timeout"));
  uint32_t maxResults = std::stoul(getParameter("maxResults"));
  
  // Phase 0: Simulation temps-réel
  if (elapsed < 5000) {
    // Découverte progressive
    if (discoveredItems < maxResults && elapsed % 1500 == 0) {
      discoveredItems++;
      
      // Créer résultat simulé
      AttackResult* result = ResultBuilder::createScan(
        "Item_" + std::to_string(discoveredItems),
        -50  // RSSI
      );
      addResult(result);
      
      Logger::getInstance().info(getName(), 
        "Trouvé: Item_" + std::to_string(discoveredItems));
    }
  }
  
  // Phase 1: Remplacer par appels matériel
  /*
  if (elapsed < 5000) {
    std::vector<HardwareResult> items = driver->scan();
    for (const auto& item : items) {
      AttackResult* result = ResultBuilder::createScan(
        item.name,
        item.rssi
      );
      addResult(result);
    }
  }
  */
  
  // Vérifier timeout
  if (elapsed > timeout) {
    setStatus(AttackStatus::SUCCESS);
    isRunning = false;
  }
}

void MyNewAttack::stop() {
  Logger::getInstance().info(getName(), 
    "Arrêt - Résultats: " + std::to_string(getResultCount()));
  
  // Phase 1: Nettoyer ressources matériel
  // driver->stop();
  
  isRunning = false;
}
```

### Étape 3 : Enregistrer dans le Catalog

Modifier `src/attack_catalog.cpp` et ajouter :

```cpp
// Dans createWiFiAttack() par exemple
case 8:
  return new MyNewAttack();
```

Et dans `getAttackNameByIndex()` :

```cpp
case 8:
  return "Ma Nouvelle Attaque";
```

---

## 🔧 Utiliser les Patterns Disponibles

### Pattern 1 : Découverte Progressive (Scanner)

```cpp
void MyNewAttack::update() {
  uint32_t elapsed = millis() - startTime;
  
  // Trouver un nouvel élément tous les 1500ms
  if (elapsed % 1500 == 0 && discoveredItems < maxResults) {
    discoveredItems++;
    AttackResult* result = ResultBuilder::createScan("Item", -50);
    addResult(result);
  }
  
  if (elapsed > timeout) {
    setStatus(AttackStatus::SUCCESS);
  }
}
```

### Pattern 2 : Capture de Paquets

```cpp
void MyNewAttack::update() {
  uint32_t elapsed = millis() - startTime;
  
  // Générer un paquet tous les 100ms
  if (elapsed % 100 == 0 && packetCount < maxPackets) {
    packetCount++;
    
    uint8_t data[] = {0xAA, 0xBB, 0xCC};
    AttackResult* result = ResultBuilder::createPacket(
      data, 
      3,
      "Paquet " + std::to_string(packetCount)
    );
    addResult(result);
  }
  
  if (elapsed > timeout) {
    setStatus(AttackStatus::PARTIAL);  // Ou SUCCESS
  }
}
```

### Pattern 3 : Attaque Avec Progression

```cpp
void MyNewAttack::update() {
  uint32_t elapsed = millis() - startTime;
  uint8_t progress = (elapsed * 100) / timeout;  // 0-100%
  
  if (progress % 10 == 0) {  // Chaque 10%
    AttackResult* result = ResultBuilder::createStatus(
      "Progression: " + std::to_string(progress) + "%"
    );
    addResult(result);
  }
  
  if (elapsed > timeout) {
    setStatus(AttackStatus::SUCCESS);
  }
}
```

---

## 📊 Utiliser ResultBuilder

Le ResultBuilder normalise la création de résultats :

### Scan (réseau, appareil, service)

```cpp
AttackResult* result = ResultBuilder::createScan(
  "NetworkName",           // Nom/description
  -50                      // RSSI/signal
);
addResult(result);
```

### Paquet (capture, sniffing)

```cpp
uint8_t payload[] = {0xDE, 0xAD, 0xBE, 0xEF};
AttackResult* result = ResultBuilder::createPacket(
  payload,                 // Données
  4,                       // Longueur
  "Paquet découvert"       // Description
);
addResult(result);
```

### Code (brute force, credentials)

```cpp
AttackResult* result = ResultBuilder::createCode(
  "SecretCode123"          // Code trouvé
);
addResult(result);
```

### Statut (messages d'info)

```cpp
AttackResult* result = ResultBuilder::createStatus(
  "Scan en cours..."       // Message
);
addResult(result);
```

---

## 🪵 Logging Integration

Toujours utiliser le Logger pour tracer l'exécution :

```cpp
// Utiliser le Logger singleton
Logger::getInstance().info(getName(), "Message d'info");
Logger::getInstance().warn(getName(), "Attention");
Logger::getInstance().error(getName(), "Erreur");

// Dans update()
if (discovered > lastLogged) {
  lastLogged = discovered;
  Logger::getInstance().info(getName(), 
    "Trouvé " + std::to_string(discovered) + " items");
}
```

---

## 🔗 Intégration Matériel Phase 1

### Exemple : WiFi Scanner Réel

```cpp
void WiFiNetworkScanReal::update() {
  uint32_t elapsed = millis() - startTime;
  
  if (elapsed % 500 == 0) {
    // Phase 1: Appeler le vrai driver WiFi
    int n = WiFi.scanNetworks(false);  // Scan asynchrone
    
    for (int i = 0; i < n; i++) {
      int rssi = WiFi.RSSI(i);
      String ssid = WiFi.SSID(i);
      
      AttackResult* result = ResultBuilder::createScan(
        ssid.c_str(),
        rssi
      );
      addResult(result);
    }
  }
  
  if (elapsed > timeout) {
    setStatus(AttackStatus::SUCCESS);
  }
}
```

### Exemple : RF Scanner Réel

```cpp
void CC1101ScannerReal::begin() {
  // Initialiser le driver CC1101
  cc1101Driver->init();
  cc1101Driver->setFrequency(433920000);  // 433.92 MHz
}

void CC1101ScannerReal::update() {
  uint32_t elapsed = millis() - startTime;
  
  if (cc1101Driver->hasData()) {
    uint8_t data[64];
    int len = cc1101Driver->receive(data);
    
    AttackResult* result = ResultBuilder::createPacket(
      data,
      len,
      "RF Paquet " + std::to_string(getResultCount())
    );
    addResult(result);
  }
  
  if (elapsed > timeout) {
    setStatus(AttackStatus::SUCCESS);
  }
}

void CC1101ScannerReal::stop() {
  cc1101Driver->stop();
}
```

---

## 📝 Exemple Complet : Nouveau Scanner BLE

### Fichier: `include/ble_hci_scanner.h`

```cpp
#ifndef BLE_HCI_SCANNER_H
#define BLE_HCI_SCANNER_H

#include "attack_framework.h"
#include <BLEDevice.h>

class BLEHCIScanner : public Attack {
public:
  BLEHCIScanner();
  
  void begin() override;
  void start() override;
  void update() override;
  void stop() override;
  
private:
  uint32_t startTime;
  uint16_t deviceCount;
};

#endif
```

### Fichier: `src/ble_hci_scanner.cpp`

```cpp
#include "ble_hci_scanner.h"
#include "logging_system.h"
#include "results_builder.h"

BLEHCIScanner::BLEHCIScanner()
  : Attack("BLE HCI Scanner", "Analyse des paquets BLE bruts"),
    startTime(0),
    deviceCount(0) {
  setCategory("BLE");
  setParameter("timeout", "20000");
  setParameter("channels", "37,38,39");
}

void BLEHCIScanner::begin() {
  // Phase 1: Initialiser BLE en mode HCI
  BLEDevice::init("ESP32-HCI");
  setStatus(AttackStatus::SCANNING);
}

void BLEHCIScanner::start() {
  startTime = millis();
  Logger::getInstance().info(getName(), "Scan HCI démarré");
  setStatus(AttackStatus::ATTACKING);
  isRunning = true;
}

void BLEHCIScanner::update() {
  if (!isRunning) return;
  
  uint32_t elapsed = millis() - startTime;
  
  // Simulation (Phase 0)
  if (elapsed % 2000 == 0 && deviceCount < 10) {
    deviceCount++;
    
    char name[32];
    snprintf(name, 31, "Device_%02X%02X%02X", 
      (deviceCount * 13) % 256,
      (deviceCount * 17) % 256,
      (deviceCount * 19) % 256
    );
    
    AttackResult* result = ResultBuilder::createScan(name, -60);
    addResult(result);
  }
  
  if (elapsed > 20000) {
    setStatus(AttackStatus::SUCCESS);
    isRunning = false;
  }
}

void BLEHCIScanner::stop() {
  Logger::getInstance().info(getName(), 
    "Scan terminé - Appareils: " + std::to_string(deviceCount));
  
  // Phase 1: BLEDevice::deinit()
  isRunning = false;
}
```

---

## ✅ Checklist pour Ajouter une Attaque

- [ ] Créer header `.h` avec classe dérivant Attack
- [ ] Implémenter `begin()`, `start()`, `update()`, `stop()`
- [ ] Utiliser `setStatus()` pour suivre l'état
- [ ] Utiliser `ResultBuilder` pour créer des résultats
- [ ] Utiliser `Logger::getInstance()` pour tracer
- [ ] Ajouter `setParameter()` pour configuration
- [ ] Enregistrer dans `AttackCatalog::createXxxAttack()`
- [ ] Ajouter dans `getAttackNameByIndex()`
- [ ] Tester avec framework de tests
- [ ] Mesurer avec benchmark

---

## 🔄 Migration Phase 0 → Phase 1

### Phase 0 (Maintenant)
```cpp
// Simulation avec millis()
if (elapsed % 1500 == 0) {
  found++;
  AttackResult* result = ResultBuilder::createScan("Item", -50);
}
```

### Phase 1 (Avec Hardware)
```cpp
// Remplacement direct
std::vector<RealItem> items = driver->scan();
for (const auto& item : items) {
  AttackResult* result = ResultBuilder::createScan(item.name, item.rssi);
}
```

**Avantage:** L'interface Attack reste identique - seule l'implémentation de `update()` change.

---

## 📚 Ressources

- `ARCHITECTURE.md` - Design global du système
- `attack_framework.h` - Classe de base Attack
- `results_builder.h` - API pour créer résultats
- `attack_catalog.h` - Enregistrement des attaques
- `logging_system.h` - Système de logging

---

## 🚀 Prochaines Étapes (Phase 1)

1. Remplacer chaque `update()` simulé par appels matériel réels
2. Valider timing avec mesures réelles
3. Optimiser utilisation mémoire (heap/PSRAM)
4. Intégrer avec UI menu pour sélection d'attaques
5. Ajouter présets de configuration par attaque

---

**Date:** 2026-09-27  
**Version:** Phase 0 Integration Guide v1.0  
**Statut:** Prêt pour Phase 1
