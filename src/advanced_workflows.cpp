#include "advanced_workflows.h"
#include "logging_system.h"

AdvancedWorkflows::AdvancedWorkflows() {
  Logger::getInstance().info("Workflows", "Workflows avancés initialisés");
}

// ============= WIFI WORKFLOWS =============

AttackWorkflow* AdvancedWorkflows::createFullWiFiReconChain() {
  AttackWorkflow* wf = new AttackWorkflow("WiFi Reconnaissance Complète");
  AttackCatalog& catalog = AttackCatalog::getInstance();

  Serial.println("📋 Workflow: WiFi Reconnaissance Complète");
  Serial.println("  Phase 1: Scan réseau");
  wf->addStep(catalog.createWiFiAttack(0), 15000, true); // WiFiNetworkScan

  Serial.println("  Phase 2: Capture handshake");
  Attack* handshake = catalog.createWiFiAttack(4); // WiFiHandshakeCapture
  handshake->setParameter("timeout", "60000");
  wf->addStep(handshake, 60000, true);

  return wf;
}

AttackWorkflow* AdvancedWorkflows::createWiFiExploitChain() {
  AttackWorkflow* wf = new AttackWorkflow("WiFi Chaîne Exploitation");
  AttackCatalog& catalog = AttackCatalog::getInstance();

  Serial.println("📋 Workflow: WiFi Exploitation");
  Serial.println("  Phase 1: Scan réseau");
  wf->addStep(catalog.createWiFiAttack(0), 15000, true); // WiFiNetworkScan

  Serial.println("  Phase 2: Déauthentification");
  Attack* deauth = catalog.createWiFiAttack(1); // WiFiDeauthAttack
  deauth->setParameter("channel", "6");
  wf->addStep(deauth, 5000, true);

  Serial.println("  Phase 3: Capture PMKID");
  wf->addStep(catalog.createWiFiAttack(3), 30000, true); // WiFiPMKIDCapture

  return wf;
}

AttackWorkflow* AdvancedWorkflows::createWiFiJammingChain() {
  AttackWorkflow* wf = new AttackWorkflow("WiFi Jamming + Beacon Flood");
  AttackCatalog& catalog = AttackCatalog::getInstance();

  Serial.println("📋 Workflow: WiFi Jamming");
  Serial.println("  Phase 1: Inondation Beacon");
  wf->addStep(catalog.createWiFiAttack(2), 10000, true); // WiFiBeaconFlood

  Serial.println("  Phase 2: Jamming");
  wf->addStep(catalog.createWiFiAttack(6), 10000, true); // WiFiJamming

  return wf;
}

// ============= BLE WORKFLOWS =============

AttackWorkflow* AdvancedWorkflows::createBLEExploitChain() {
  AttackWorkflow* wf = new AttackWorkflow("BLE Chaîne Exploitation");
  AttackCatalog& catalog = AttackCatalog::getInstance();

  Serial.println("📋 Workflow: BLE Exploitation");
  Serial.println("  Phase 1: Scan BLE");
  wf->addStep(catalog.createBLEAttack(0), 15000, true); // BLEScanner

  Serial.println("  Phase 2: Énumération GATT");
  wf->addStep(catalog.createBLEAttack(3), 20000, true); // BLEGATTEnumeration

  Serial.println("  Phase 3: Relecture appairage");
  wf->addStep(catalog.createBLEAttack(4), 15000, true); // BLEPairingReplay

  return wf;
}

AttackWorkflow* AdvancedWorkflows::createBLESweepChain() {
  AttackWorkflow* wf = new AttackWorkflow("BLE Multi-Canal Sweep");
  AttackCatalog& catalog = AttackCatalog::getInstance();

  Serial.println("📋 Workflow: BLE Multi-Canal");
  Serial.println("  Phase 1: Balayage tous canaux");
  wf->addStep(catalog.createBLEAttack(5), 30000, true); // BLESweeper

  Serial.println("  Phase 2: Interception paquets");
  wf->addStep(catalog.createBLEAttack(6), 20000, true); // BLESniffer

  return wf;
}

// ============= RF WORKFLOWS =============

AttackWorkflow* AdvancedWorkflows::createMultiBandRFChain() {
  AttackWorkflow* wf = new AttackWorkflow("Chaîne RF Multi-Bande");
  AttackCatalog& catalog = AttackCatalog::getInstance();

  Serial.println("📋 Workflow: RF Multi-Bande");
  Serial.println("  Phase 1: Scan NRF24 2.4GHz");
  wf->addStep(catalog.createRFAttack(0), 10000, true); // NRF24Scanner

  Serial.println("  Phase 2: Scan CC1101 433MHz");
  wf->addStep(catalog.createRFAttack(2), 15000, true); // CC1101Scanner

  Serial.println("  Phase 3: Interception LoRa 868MHz");
  wf->addStep(catalog.createRFAttack(6), 25000, true); // LoRaSniffer

  Serial.println("  Phase 4: Balayage ISM multi-bande");
  wf->addStep(catalog.createRFAttack(7), 30000, true); // ISMBandSweeper

  return wf;
}

// ============= IOT WORKFLOWS =============

AttackWorkflow* AdvancedWorkflows::createIoTAuditChain() {
  AttackWorkflow* wf = new AttackWorkflow("Audit IoT Maison Intelligente");
  AttackCatalog& catalog = AttackCatalog::getInstance();

  Serial.println("📋 Workflow: Audit IoT Complet");
  Serial.println("  Phase 1: Découverte appareils");
  wf->addStep(catalog.createNFCAttack(6), 30000, true); // SmartHomeScanner

  Serial.println("  Phase 2: Interception Zigbee");
  wf->addStep(catalog.createNFCAttack(4), 20000, true); // ZigbeeSniffer

  Serial.println("  Phase 3: Interception MQTT");
  Attack* mqtt = catalog.createNFCAttack(5); // MQTTInterceptor
  mqtt->setParameter("broker", "192.168.1.1");
  wf->addStep(mqtt, 15000, true);

  Serial.println("  Phase 4: Scan vulnérabilités");
  wf->addStep(catalog.createAdvancedAttack(6), 30000, true); // VulnerabilityScanner

  return wf;
}

// ============= COMBINED WORKFLOWS =============

AttackWorkflow* AdvancedWorkflows::createMultiBandCombinedChain() {
  AttackWorkflow* wf = new AttackWorkflow("Scan Multi-Bande Parallèle");
  AttackCatalog& catalog = AttackCatalog::getInstance();

  Serial.println("📋 Workflow: Multi-Bande Parallèle");
  Serial.println("  Phase 1: Triple scan simultané");
  Serial.println("    - WiFi scan");
  wf->addStep(catalog.createWiFiAttack(0), 15000, false); // WiFiNetworkScan (non-blocking)

  Serial.println("    - BLE scan");
  wf->addStep(catalog.createBLEAttack(0), 15000, false); // BLEScanner (non-blocking)

  Serial.println("    - RF scan");
  wf->addStep(catalog.createRFAttack(0), 10000, false); // NRF24Scanner (non-blocking)

  return wf;
}

AttackWorkflow* AdvancedWorkflows::createFullSecurityAudit() {
  AttackWorkflow* wf = new AttackWorkflow("Audit Sécurité Complet");
  AttackCatalog& catalog = AttackCatalog::getInstance();

  Serial.println("📋 Workflow: Audit Sécurité Complet");

  Serial.println("  Étape 1: Reconnaissance");
  wf->addStep(catalog.createWiFiAttack(0), 15000, true);
  wf->addStep(catalog.createBLEAttack(0), 15000, true);

  Serial.println("  Étape 2: Capture données");
  wf->addStep(catalog.createWiFiAttack(4), 60000, true);
  wf->addStep(catalog.createBLEAttack(3), 20000, true);

  Serial.println("  Étape 3: Tests exploitation");
  Attack* deauth = catalog.createWiFiAttack(1);
  deauth->setParameter("channel", "6");
  wf->addStep(deauth, 5000, true);

  Serial.println("  Étape 4: Espionnage");
  wf->addStep(catalog.createAdvancedAttack(0), 25000, true); // PacketSniffer
  wf->addStep(catalog.createAdvancedAttack(1), 20000, true); // MITM

  Serial.println("  Étape 5: Analyse vulnérabilités");
  wf->addStep(catalog.createAdvancedAttack(6), 30000, true); // VulnerabilityScanner

  return wf;
}
