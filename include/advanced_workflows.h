#ifndef ADVANCED_WORKFLOWS_H
#define ADVANCED_WORKFLOWS_H

#include "attack_templates.h"

// ============= ADVANCED WORKFLOWS =============
class AdvancedWorkflows {
public:
  static AdvancedWorkflows& getInstance() {
    static AdvancedWorkflows instance;
    return instance;
  }

  // WiFi chaînes complètes
  AttackWorkflow* createFullWiFiReconChain();      // Scan + Handshake
  AttackWorkflow* createWiFiExploitChain();        // Deauth + Capture
  AttackWorkflow* createWiFiJammingChain();        // Jamming + Beacon

  // BLE chaînes
  AttackWorkflow* createBLEExploitChain();         // Scan + GATT + Pair
  AttackWorkflow* createBLESweepChain();           // Multi-channel sweep

  // RF chaînes
  AttackWorkflow* createMultiBandRFChain();        // NRF24 + CC1101 + LoRa

  // IoT chaînes
  AttackWorkflow* createIoTAuditChain();           // Smart home complet

  // Chaînes combinées
  AttackWorkflow* createMultiBandCombinedChain();  // WiFi + BLE + RF parallèle
  AttackWorkflow* createFullSecurityAudit();       // Audit sécurité complet

private:
  AdvancedWorkflows();
};

#endif // ADVANCED_WORKFLOWS_H
