# Architecture Système: Plateforme Offensive Security ESP32-S3

## 📊 Vue d'Ensemble Globale

```
┌─────────────────────────────────────────────────────────────────┐
│                    PLATEFORME ESP32-S3                          │
│                (16MB Flash, 8MB PSRAM)                           │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                      COUCHE UI/MENU                              │
│  ┌────────────────┐  ┌─────────────┐  ┌──────────────────┐     │
│  │ UIStateMachine │→ │ MenuItem    │→ │ AttackSelection  │     │
│  │ (FSM)          │  │ Callbacks   │  │ Menu             │     │
│  └────────────────┘  └─────────────┘  └──────────────────┘     │
└─────────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│                   COUCHE ORCHESTRATION                           │
│  ┌──────────────────────┐      ┌─────────────────┐             │
│  │ AttackCatalog        │      │ AttackOrchestra │             │
│  │ - createAttack()     │  →   │ - allocResource │             │
│  │ - listByCategory()   │      │ - detectConflict│             │
│  │ - 43 attaques        │      │ - executeMulti  │             │
│  └──────────────────────┘      └─────────────────┘             │
└─────────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│                  COUCHE FRAMEWORK/PATTERNS                       │
│                                                                  │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │              ATTACK BASE CLASS                          │  │
│  │  ┌──────────┬───────────┬────────┬──────────┐          │  │
│  │  │ begin()  │ start()   │update()│ stop()   │          │  │
│  │  └──────────┴───────────┴────────┴──────────┘          │  │
│  │  ┌──────────────────────────────────────────┐          │  │
│  │  │ Results[] | Status | Parameters | Logger│          │  │
│  │  └──────────────────────────────────────────┘          │  │
│  └─────────────────────────────────────────────────────────┘  │
│                                                                  │
│  ┌─────────────────┐  ┌──────────────┐  ┌────────────────┐   │
│  │ ResultBuilder   │  │ Logger       │  │ ConfigManager  │   │
│  │ - createScan()  │  │ - logStart() │  │ - getProfile() │   │
│  │ - createPacket()│  │ - logResult()│  │ - saveAttack() │   │
│  └─────────────────┘  └──────────────┘  └────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│              COUCHE IMPLÉMENTATIONS (43 Attaques)               │
│                                                                  │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐          │
│  │ WiFiAttacks  │  │ BLEAttacks   │  │ RFAttacks    │          │
│  │  (7)         │  │  (7)         │  │  (8)         │          │
│  └──────────────┘  └──────────────┘  └──────────────┘          │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐          │
│  │ NFCAttacks   │  │ DoSAttacks   │  │ Advanced     │          │
│  │  (7)         │  │  (6)         │  │ Attacks (8)  │          │
│  └──────────────┘  └──────────────┘  └──────────────┘          │
└─────────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│           COUCHE SUPPORT/SYSTÈMES AUXILIAIRES                   │
│                                                                  │
│  ┌──────────────────┐  ┌──────────────────┐                    │
│  │ EnergyManager    │  │ SystemDiagnostic │                    │
│  │ - powerScaling() │  │ - hardwareDetect │                    │
│  │ - batteryAlert() │  │ - selfTest()     │                    │
│  └──────────────────┘  └──────────────────┘                    │
│                                                                  │
│  ┌──────────────────┐  ┌──────────────────┐                    │
│  │ AdvancedLogging  │  │ ModuleRegistry   │                    │
│  │ - exportJSON()   │  │ - loadModule()   │                    │
│  │ - cloudSync()    │  │ - dependencies() │                    │
│  └──────────────────┘  └──────────────────┘                    │
└─────────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│                 COUCHE MATÉRIEL                                  │
│                                                                  │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐          │
│  │ GPIO         │  │ SPI          │  │ I2C          │          │
│  │ - Buttons    │  │ - CC1101     │  │ - RTC        │          │
│  │ - Buzzer     │  │ - NRF24      │  │ - NFC        │          │
│  │ - Battery    │  │ - TFT Display│  │ (Phase 1)    │          │
│  └──────────────┘  └──────────────┘  └──────────────┘          │
│                                                                  │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐          │
│  │ UART         │  │ WiFi         │  │ BLE          │          │
│  │ - GPS (Futur)│  │ - ESP32 chip │  │ - ESP32 chip │          │
│  │ (Phase 1)    │  │ (Phase 1)    │  │ (Phase 1)    │          │
│  └──────────────┘  └──────────────┘  └──────────────┘          │
└─────────────────────────────────────────────────────────────────┘
```

---

## 🔄 Cycle de Vie d'une Attaque

```
┌─────────────┐
│   Création  │
│  (Catalog)  │
└──────┬──────┘
       │ Attack* attack = catalog.createWiFiAttack(0)
       ↓
┌─────────────┐
│  begin()    │◄─── Initialisation ressources
│ Allocation  │     Config log
└──────┬──────┘
       │
       ↓
┌─────────────┐
│  start()    │◄─── Démarrage chrono (millis())
│  Lancement  │     Logging início
└──────┬──────┘
       │
       ↓
┌──────────────────────┐
│  update() loop       │◄──┐
│  millis() > timeout? │   │ (Tous les 50ms)
│  Progression?        │   │ Tant que isRunning
│  Résultats?          ├──┘
└──────┬───────────────┘
       │
       ↓ (timeout atteint ou success)
┌─────────────┐
│  stop()     │◄─── Cleanup ressources
│ Arrêt       │     Logger final
└──────┬──────┘
       │
       ↓
┌──────────────────────┐
│ Résultats dispo      │
│ - getResultCount()   │
│ - getResult(i)       │
│ - getCurrentStatus() │
└──────────────────────┘
```

---

## 🏗️ Pattern: Modèle Fabrique Centralisée

```
AttackCatalog (Singleton)
  │
  ├→ createWiFiAttack(index)
  │   └→ new WiFiNetworkScan/Deauth/etc
  │
  ├→ createBLEAttack(index)
  │   └→ new BLEScanner/Disconnect/etc
  │
  ├→ createRFAttack(index)
  │   └→ new NRF24Scanner/CC1101/etc
  │
  ├→ createNFCAttack(index)
  │   └→ new NFCTagReader/MIFARE/etc
  │
  ├→ createDOSAttack(index)
  │   └→ new FloodAttack/Amplification/etc
  │
  ├→ createAdvancedAttack(index)
  │   └→ new PacketSniffer/MITM/etc
  │
  └→ createAttackByName(name)
      └→ Recherche dans tous catalogues
```

---

## 🎯 Patterns de Conception Utilisés

### 1. Factory Pattern (Fabrique)
```
AttackCatalog::createWiFiAttack(0) → WiFiNetworkScan
AttackCatalog::createAttackByName("WiFi Deauth") → WiFiDeauthAttack
```

### 2. Singleton Pattern
```
Logger::getInstance()
ConfigManager::getInstance()
AttackCatalog::getInstance()
EnergyManager::getInstance()
```

### 3. Template Method Pattern
```
Attack base class:
  begin() → virtual (chaque classe implémente)
  start() → virtual
  update() → virtual
  stop() → virtual
```

### 4. Builder Pattern
```
ResultBuilder::createScan(ssid, rssi) → AttackResult*
ResultBuilder::createPacket(data, len, desc) → AttackResult*
```

### 5. Strategy Pattern
```
AttackStatus enum: SUCCESS/FAILED/PARTIAL/ERROR
Chaque attaque choisit sa stratégie de succès
```

### 6. FSM Pattern (État Machine)
```
UIStateMachine:
  STARTUP → MAIN_MENU → ATTACK_MENU → ATTACK_CONFIG
  → ATTACK_RUNNING → ATTACK_RESULTS → MAIN_MENU
```

---

## 💾 Modèle de Données: AttackResult

```cpp
struct AttackResult {
  // Identification
  ResultType type;           // SCAN, PACKET, CODE, STATUS, ERROR
  uint32_t timestamp;        // millis() création
  
  // Données
  char data[256];            // Description/données
  uint8_t* rawData;          // Données brutes (optionnel)
  uint16_t dataLength;       // Taille données brutes
  
  // Contexte
  int8_t rssi;               // Signal strength (-100 à 0)
  uint8_t frequency;         // Canal/fréquence
  uint16_t resultIndex;      // Numéro dans workflow
};
```

---

## 🔌 Flux d'Intégration Ressources

```
AttackOrchestrator (ResourceManager)
  │
  ├→ GPIO Pool
  │   ├─ Buttons (4)
  │   ├─ Buzzer (1)
  │   ├─ Battery ADC
  │   └─ IR RX/TX
  │
  ├→ I2C Bus
  │   ├─ RTC DS3231
  │   └─ PN532 NFC
  │
  ├→ SPI Bus
  │   ├─ Display TFT
  │   ├─ CC1101 (433MHz)
  │   └─ NRF24 (2.4GHz)
  │
  ├→ UART
  │   └─ GPS NEO-6M (Phase 1)
  │
  ├→ WiFi Driver
  │   └─ Scans/Deauth/Jamming
  │
  └→ BLE Driver
      └─ Scans/GATT/Pairing
```

---

## 📊 Workflow Orchestration

```
WorkflowStep[] steps
  ├─ [0] Attack* + timeout + waitFlag
  ├─ [1] Attack* + timeout + waitFlag
  ├─ [2] Attack* + timeout + waitFlag
  └─ [n] Attack* + timeout + waitFlag

Start workflow:
  1. Initialize steps[0]->attack
  2. Start steps[0]->attack
  3. Update steps[0]->attack in loop
  4. If timeout OR completion: stop + go to step[1]
  5. Repeat until all steps done
  6. Aggregated results from all steps
```

---

## 🌐 Simulation Temps-Réel

Toutes les 43 attaques utilisent:

```cpp
uint32_t startTime = millis();  // Au démarrage
uint32_t elapsed = millis() - startTime;

while (isRunning) {
  update() {
    elapsed = millis() - startTime;
    
    // Découverte progressive
    if (elapsed < 5000) {
      if (networksFound < 3 && elapsed % 1500 == 0) {
        networksFound++;
        createResult();  // Tous les 1500ms
      }
    }
    
    // Terminer après timeout
    if (elapsed > SCAN_TIMEOUT) {
      setStatus(SUCCESS);
      isRunning = false;
    }
  }
  delay(50);
}
```

**Avantages:**
- Progression réaliste sans matériel réel
- Timing reproductible (millis() ESP32)
- Phase 1: Remplacer elapsed simulation par mesures réelles

---

## 🎛️ Configuration et Persistance

```
ConfigManager (Singleton)
  │
  ├─ SystemConfig (NVS Storage)
  │   ├─ Device name
  │   ├─ WiFi credentials
  │   └─ Auto-resume flag
  │
  ├─ UserProfile (5 profils)
  │   ├─ PERFORMANCE (240MHz)
  │   ├─ BALANCED (80MHz)
  │   ├─ STEALTH (40MHz)
  │   └─ Custom
  │
  ├─ AttackPreset (Sauvegardables)
  │   ├─ wifi_pentest.json
  │   ├─ ble_exploit.json
  │   └─ ...
  │
  └─ Dynamic Parameters
      └─ setParameter(key, value)
```

---

## 🧪 Testing et Validation

```
AttackTestSuite
  ├─ runAllTests()
  │   ├─ testWiFiAttacks()     (7 tests)
  │   ├─ testBLEAttacks()      (7 tests)
  │   ├─ testRFAttacks()       (8 tests)
  │   ├─ testNFCAttacks()      (7 tests)
  │   ├─ testDoSAttacks()      (6 tests)
  │   └─ testAdvancedAttacks() (8 tests)
  │
  └─ Results
      ├─ printSummary()         (Passé/Échoué)
      ├─ printDetailedReport()  (Table complète)
      └─ exportResultsToCSV()   (Pour analyse)

PerformanceBenchmark
  ├─ benchmarkAllAttacks()
  │   ├─ Duration (min/max/avg)
  │   ├─ Heap usage
  │   ├─ Efficacité (résultats/ms)
  │   └─ Comparaisons
  │
  └─ Export
      └─ exportToCSV()
```

---

## 📈 Logging Hiérarchique

```
Logger (Singleton)
  │
  ├─ logAttackStart(name, msg)
  │   └─ [INFO] [WiFi] Démarrage scan...
  │
  ├─ logResult(category, result)
  │   └─ [INFO] [WiFi] Résultat: Network_1 (-50dBm)
  │
  ├─ logAttackEnd(name, status)
  │   └─ [INFO] [WiFi] Termined (SUCCESS)
  │
  ├─ info(cat, msg)
  │   └─ [INFO] [Category] Message
  │
  ├─ warn(cat, msg)
  │   └─ [WARN] [Category] Message
  │
  └─ error(cat, msg)
      └─ [ERROR] [Category] Message
```

---

## 🚀 Roadmap: Phase 0 → Phase 1

### Phase 0 (Complétée) ✓
- ✅ 43 attaques simulées
- ✅ 8 frameworks systémiques
- ✅ Documentation exhaustive
- ✅ Tests unitaires
- ✅ Benchmarks performance

### Phase 1 (Matériel) 🔧
- [ ] Drivers matériel réels (CC1101, NRF24, PN532)
- [ ] Validation WiFi/BLE/RF réel
- [ ] Timing tuning
- [ ] Mémoire optimization

### Phase 2 (Avancé) 🎯
- [ ] Web dashboard
- [ ] Cloud sync
- [ ] OTA updates
- [ ] Machine learning

---

**Date**: 2026-09-27  
**Version**: Architecture v1.0  
**Phase**: Phase 0 Complétée
