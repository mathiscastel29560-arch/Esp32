#include "zwave_scanner.h"
#include <vector>

namespace ZwaveScanner {

static std::vector<ZwaveNode> discoveredNodes;

ScanResult scanZwaveNetwork(uint32_t durationMs) {
    ScanResult result = {false, 0, 0, -100, 1};
    discoveredNodes.clear();

    uint32_t startTime = millis();
    int8_t strongestRssi = -100;
    uint32_t nodeCount = 0;

    // Z-Wave Home ID discovery and node enumeration
    // Scan typically covers 868.4 MHz (EU) or 915 MHz (US/AU)

    while (millis() - startTime < durationMs) {
        // Simulate finding Z-Wave nodes
        if ((esp_random() % 100) < 20) {
            ZwaveNode node;
            node.nodeId = ((esp_random() % 230) + 2);  // Z-Wave node IDs 2-231 (1=controller)
            node.rssi = -30 - (esp_random() % 50);
            node.securityLevel = (esp_random() % 3);  // 0=none, 1=S0, 2=S2
            node.timestamp = millis();

            // Classify device type
            uint8_t devType = (esp_random() % 6);
            switch(devType) {
                case 0: node.deviceType = "SmartLock"; break;
                case 1: node.deviceType = "SmartSwitch"; break;
                case 2: node.deviceType = "Thermostat"; break;
                case 3: node.deviceType = "Sensor"; break;
                case 4: node.deviceType = "DoorLock"; break;
                default: node.deviceType = "Generic"; break;
            }

            // Manufacturer simulation
            const char* manufacturers[] = {"Aeotec", "Fibaro", "Danfoss", "Qubino", "RGBgenie"};
            node.manufacturer = manufacturers[(esp_random() % 5)];

            discoveredNodes.push_back(node);
            nodeCount++;

            if (node.rssi > strongestRssi) {
                strongestRssi = node.rssi;
            }
        }
        delay(100);
    }

    result.success = (nodeCount > 0);
    result.nodeCount = nodeCount;
    result.durationMs = millis() - startTime;
    result.strongestRssi = strongestRssi;
    result.controllerNode = 1;  // Z-Wave controller is always node 1

    return result;
}

const ZwaveNode* getDiscoveredNodes(uint32_t& outCount) {
    outCount = discoveredNodes.size();
    return discoveredNodes.empty() ? nullptr : discoveredNodes.data();
}

InjectionResult injectZwaveCommands(uint8_t targetNode, uint32_t durationMs, const char* cmdType) {
    InjectionResult result = {false, 0, 0, ""};

    uint32_t startTime = millis();
    uint32_t commandsSent = 0;
    String type = String(cmdType);

    // Z-Wave command injection types
    while (millis() - startTime < durationMs) {
        if (type == "BASIC_SET") {
            // Basic On/Off commands
            commandsSent += ((esp_random() % 10) + 5);
        } else if (type == "SWITCH_MULTILEVEL") {
            // Dimming commands
            commandsSent += ((esp_random() % 7) + 3);
        } else if (type == "LOCK_CONTROL") {
            // Door lock commands
            commandsSent += ((esp_random() % 6) + 2);
        } else if (type == "THERMOSTAT") {
            // Temperature control
            commandsSent += ((esp_random() % 4) + 2);
        } else {
            commandsSent += ((esp_random() % 8) + 4);
        }
        delay(200);
    }

    result.success = (commandsSent > 0);
    result.commandsSent = commandsSent;
    result.durationMs = millis() - startTime;
    result.commandType = type;

    return result;
}

SecurityBypassResult bypassZwaveSecurity(uint32_t durationMs) {
    SecurityBypassResult result = {false, 0, 0, ""};

    uint32_t startTime = millis();
    uint32_t attempts = 0;

    // S0/S2 security bypass attempts
    const char* vulnerabilities[] = {
        "S0_Replay_Attack",
        "S2_ECDH_Bypass",
        "Insecure_Inclusion",
        "Nonce_Prediction",
        "Key_Derivation_Weak"
    };

    while (millis() - startTime < durationMs) {
        attempts++;

        // Simulate occasional successful bypass
        if (attempts > 500 && (esp_random() % 100) < 3) {
            result.success = true;
            result.vulnerabilityFound = vulnerabilities[(esp_random() % 5)];
            break;
        }
        delay(10);
    }

    result.attemptCount = attempts;
    result.durationMs = millis() - startTime;

    return result;
}

KeyRecoveryResult recoverZwaveNetworkKey(uint32_t durationMs) {
    KeyRecoveryResult result = {false, "", 0};

    uint32_t startTime = millis();

    // Simulate network key recovery via S0 desynchronization attack
    // or by monitoring insecure inclusion

    while (millis() - startTime < durationMs) {
        // Low probability of successful key recovery
        if ((esp_random() % 100) < 2) {
            char keyBuf[33] = {0};
            snprintf(keyBuf, sizeof(keyBuf), "%08X%08X%08X%08X",
                    (esp_random() % 4294967295), (esp_random() % 4294967295),
                    (esp_random() % 4294967295), (esp_random() % 4294967295));
            result.networkKey = String(keyBuf);
            result.success = true;
            break;
        }
        delay(50);
    }

    result.durationMs = millis() - startTime;

    return result;
}

ZwaveStats getZwaveStats() {
    ZwaveStats stats = {0, 0, 0, 15, 0};

    if (discoveredNodes.empty()) {
        return stats;
    }

    stats.totalNodesFound = discoveredNodes.size();

    float rssiSum = 0;
    for (const auto& node : discoveredNodes) {
        rssiSum += node.rssi;

        if (node.securityLevel == 2) {
            stats.secureNodes++;
        } else if (node.securityLevel == 0) {
            stats.vulnerableNodes++;
        }
    }

    stats.averageRssi = rssiSum / discoveredNodes.size();
    stats.networkChannel = 15;  // Z-Wave EU868 channel

    return stats;
}

}  // namespace ZwaveScanner
