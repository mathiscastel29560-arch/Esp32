#include "result_renderers.h"

namespace ResultRenderers {

void renderWiFiScan(const WiFiScanResult &result) {
    Serial.println("\n==================================================");
    Serial.println("📡 WiFi SCAN RESULTS");
    Serial.println("==================================================");

    Serial.printf("Networks Found:  %u\n", result.devicesFound);
    Serial.printf("Strongest SSID:  %s\n", result.strongestSSID.c_str());
    Serial.printf("Signal Strength: %d dBm\n", result.strongestRssi);
    Serial.printf("Duration:        %lums\n", result.durationMs);

    Serial.println("\n📊 Channel Distribution:");
    for (uint8_t ch = 1; ch <= 14 && ch < result.channelDistribution.size(); ch++) {
        uint8_t count = result.channelDistribution[ch - 1];
        if (count > 0) {
            String bar = "";
            for (uint8_t i = 0; i < count; i++) bar += "█";
            Serial.printf("  Ch %2u: %s (%u)\n", ch, bar.c_str(), count);
        }
    }

    if (!result.allRssiValues.empty()) {
        uint16_t weak = 0, fair = 0, good = 0, excellent = 0;
        for (int32_t rssi : result.allRssiValues) {
            if (rssi < -80) weak++;
            else if (rssi < -60) fair++;
            else if (rssi < -40) good++;
            else excellent++;
        }
        Serial.println("\n📈 Signal Strength:");
        Serial.printf("  🔴 Weak (<-80dBm):     %u\n", weak);
        Serial.printf("  🟡 Fair (-80/-60dBm):  %u\n", fair);
        Serial.printf("  🟢 Good (-60/-40dBm):  %u\n", good);
        Serial.printf("  🟩 Excellent (>-40dBm): %u\n", excellent);
    }
    Serial.println();
}

void renderBLEScan(const BLEScanResult &result) {
    Serial.println("\n==================================================");
    Serial.println("📱 BLUETOOTH SCAN RESULTS");
    Serial.println("==================================================");

    Serial.printf("Devices Found:   %u\n", result.devicesFound);
    Serial.printf("Paired Devices:  %u\n", result.pairedDevices);
    Serial.printf("Strongest Device:%s\n", result.strongestDevice.c_str());
    Serial.printf("Signal Strength: %d dBm\n", result.strongestRssi);
    Serial.printf("Duration:        %lums\n", result.durationMs);

    if (!result.allRssiValues.empty()) {
        Serial.println("\n📊 RSSI Distribution:");
        uint16_t veryWeak = 0, weak = 0, fair = 0, strong = 0;
        for (int32_t rssi : result.allRssiValues) {
            if (rssi < -90) veryWeak++;
            else if (rssi < -70) weak++;
            else if (rssi < -50) fair++;
            else strong++;
        }
        Serial.printf("  Very Weak (<-90dBm):  %u ▁\n", veryWeak);
        Serial.printf("  Weak (-90/-70dBm):    %u ▂\n", weak);
        Serial.printf("  Fair (-70/-50dBm):    %u ▃\n", fair);
        Serial.printf("  Strong (>-50dBm):     %u ▄\n", strong);
    }
    Serial.println();
}

void renderRFScan(const RFScanResult &result) {
    Serial.println("\n==================================================");
    Serial.println("📶 RF/SubGHz SCAN RESULTS");
    Serial.println("==================================================");

    Serial.printf("Signals Detected:  %u\n", result.signalsDetected);
    Serial.printf("Protocol:          %s\n", result.dominantProtocol.c_str());
    Serial.printf("Rolling Codes:     %u\n", result.rollingCodesDetected);
    Serial.printf("Duration:          %lums\n", result.durationMs);

    if (!result.frequencies.empty()) {
        Serial.println("\n📊 Active Frequencies:");
        for (size_t i = 0; i < result.frequencies.size() && i < 10; i++) {
            Serial.printf("  %lu MHz - Signal: %d dBm\n",
                         result.frequencies[i], result.signalStrengths[i]);
        }
    }
    Serial.println();
}

void renderNFCScan(const NFCScanResult &result) {
    Serial.println("\n==================================================");
    Serial.println("🏷️  NFC/RFID SCAN RESULTS");
    Serial.println("==================================================");

    Serial.printf("Cards Detected:         %u\n", result.cardsDetected);
    Serial.printf("Relayed Successfully:   %u\n", result.relayedSuccessfully);
    Serial.printf("Vulnerabilities Found:  %u\n", result.vulnerabilitiesFound);
    Serial.printf("Duration:               %lums\n", result.durationMs);

    uint8_t percent = (result.relayedSuccessfully * 100) / max(result.cardsDetected, 1U);
    Serial.printf("\n📊 Relay Success: [");
    for (uint8_t i = 0; i < 20; i++) {
        Serial.print(i * 5 < percent ? "█" : "░");
    }
    Serial.printf("] %u%%\n", percent);

    if (!result.cardTypes.empty()) {
        Serial.println("\n🏷️  Card Types Detected:");
        for (const String &type : result.cardTypes) {
            Serial.printf("  • %s\n", type.c_str());
        }
    }
    Serial.println();
}

void renderCellularScan(const CellularScanResult &result) {
    Serial.println("\n==================================================");
    Serial.println("📡 CELLULAR SCAN RESULTS");
    Serial.println("==================================================");

    Serial.printf("Devices Detected:  %u\n", result.devicesDetected);
    Serial.printf("IMSI Captured:     %u\n", result.imsiCaptured);
    Serial.printf("Downgrade Attacks: %u\n", result.downgradeAttacks);
    Serial.printf("Duration:          %lums\n", result.durationMs);

    uint8_t percent = (result.imsiCaptured * 100) / max(result.devicesDetected, 1U);
    Serial.printf("\n📊 IMSI Capture Rate: [");
    for (uint8_t i = 0; i < 20; i++) {
        Serial.print(i * 5 < percent ? "█" : "░");
    }
    Serial.printf("] %u%%\n", percent);

    if (!result.networkTypes.empty()) {
        Serial.println("\n🌐 Network Types Detected:");
        for (const String &net : result.networkTypes) {
            Serial.printf("  • %s\n", net.c_str());
        }
    }
    Serial.println();
}

void renderIoTScan(const IoTScanResult &result) {
    Serial.println("\n==================================================");
    Serial.println("🔧 IoT SCAN RESULTS");
    Serial.println("==================================================");

    Serial.printf("Devices Found:         %u\n", result.devicesFound);
    Serial.printf("Brokers Found:         %u\n", result.brokersFound);
    Serial.printf("Vulnerabilities:       %u\n", result.vulnerabilitiesDiscovered);
    Serial.printf("Duration:              %lums\n", result.durationMs);

    Serial.println("\n🔌 Protocols Detected:");
    for (size_t i = 0; i < result.protocols.size(); i++) {
        Serial.printf("  %u. %s (Port %u)\n", i + 1,
                     result.protocols[i].c_str(),
                     result.ports[i % result.ports.size()]);
    }
    Serial.println();
}

void renderAttackSuccess(const AttackSuccessResult &result) {
    Serial.println("\n==================================================");
    Serial.println("⚔️  ATTACK RESULTS");
    Serial.println("==================================================");

    Serial.printf("Attack Type:   %s\n", result.attackName.c_str());
    Serial.printf("Targets:       %u\n", result.targetCount);
    Serial.printf("Successful:    %u\n", result.successCount);
    Serial.printf("Failed:        %u\n", result.failureCount);
    Serial.printf("Success Rate:  %u%%\n", result.successPercent);
    Serial.printf("Duration:      %lums\n", result.durationMs);

    Serial.print("\n📊 Progress: [");
    uint8_t filled = result.successPercent / 5;
    for (uint8_t i = 0; i < 20; i++) {
        Serial.print(i < filled ? "█" : "░");
    }
    Serial.printf("] %u%%\n\n", result.successPercent);
}

void renderSignalDistribution(const SignalDistributionResult &result) {
    Serial.println("\n==================================================");
    Serial.println("📊 SIGNAL DISTRIBUTION - " + result.scanType);
    Serial.println("==================================================");

    Serial.printf("Total Signals:  %u\n", result.totalSignals);
    Serial.printf("Average RSSI:   %d dBm\n", result.averageRssi);
    Serial.printf("Min RSSI:       %d dBm\n", result.minRssi);
    Serial.printf("Max RSSI:       %d dBm\n", result.maxRssi);

    Serial.println("\n📈 Strength Bands:");
    if (result.strengthBands.size() >= 4) {
        Serial.printf("  Weak    [");
        for (uint16_t i = 0; i < result.strengthBands[0] / 10; i++) Serial.print("█");
        Serial.printf("] %u\n", result.strengthBands[0]);

        Serial.printf("  Fair    [");
        for (uint16_t i = 0; i < result.strengthBands[1] / 10; i++) Serial.print("█");
        Serial.printf("] %u\n", result.strengthBands[1]);

        Serial.printf("  Good    [");
        for (uint16_t i = 0; i < result.strengthBands[2] / 10; i++) Serial.print("█");
        Serial.printf("] %u\n", result.strengthBands[2]);

        Serial.printf("  Strong  [");
        for (uint16_t i = 0; i < result.strengthBands[3] / 10; i++) Serial.print("█");
        Serial.printf("] %u\n", result.strengthBands[3]);
    }
    Serial.println();
}

void renderExploitResults(const ExploitResult &result) {
    Serial.println("\n==================================================");
    Serial.println("💣 EXPLOIT RESULTS");
    Serial.println("==================================================");

    Serial.printf("Exploit Type:      %s\n", result.exploitType.c_str());
    Serial.printf("Attacks Executed:  %u\n", result.attacksExecuted);
    Serial.printf("Successful:        %u\n", result.successfulExploits);
    Serial.printf("Compromised:       %u\n", result.targetsCompromised);
    Serial.printf("Risk Level:        %u/100\n", result.riskLevel);

    Serial.print("\n📊 Risk: [");
    uint8_t filled = result.riskLevel / 5;
    for (uint8_t i = 0; i < 20; i++) {
        Serial.print(i < filled ? "█" : "░");
    }
    Serial.printf("] %u%%\n", result.riskLevel);

    if (!result.vulnerabilitiesFound.empty()) {
        Serial.println("\n🔓 Vulnerabilities Exploited:");
        for (const String &vuln : result.vulnerabilitiesFound) {
            Serial.printf("  ⚠️  %s\n", vuln.c_str());
        }
    }
    Serial.println();
}

void ProgressRenderer::start(const String &title, uint32_t totalSteps) {
    Serial.println("\n==================================================");
    Serial.println("⏳ " + title);
    Serial.println("==================================================");
}

void ProgressRenderer::update(uint32_t currentStep, const String &message, uint8_t percent) {
    Serial.printf("[%3u%%] %s\n", percent, message.c_str());
}

void ProgressRenderer::complete(const String &finalMessage) {
    Serial.println("✅ " + finalMessage);
    Serial.println();
}

}  // namespace ResultRenderers
