#include "nrf24_replay.h"
#include "nrf24_tools.h"
#include "config.h"
#include "tx_arm.h"
#include <RF24.h>

namespace {
RF24 radio(PIN_NRF24_CE, PIN_NRF24_CS);
}

namespace Nrf24Replay {

PacketCapture capturePacket(uint8_t channel, uint32_t timeoutMs) {
    PacketCapture pkt{channel, {}, millis(), 0};
    
    Serial.println("\n=== NRF24 Packet Capture ===");
    Serial.println("Channel: " + String(channel));
    Serial.println("Timeout: " + String(timeoutMs) + "ms");
    
    radio.begin();
    radio.setAutoAck(false);
    radio.setRetries(0, 0);
    radio.disableCRC();
    radio.setChannel(channel);
    radio.setPALevel(RF24_PA_MIN);
    radio.startListening();
    
    uint32_t start = millis();
    while (millis() - start < timeoutMs) {
        if (radio.available()) {
            uint8_t buffer[32];
            uint8_t len = radio.getDynamicPayloadSize();
            if (len > 32) len = 32;
            radio.read(buffer, len);
            
            pkt.data.assign(buffer, buffer + len);
            pkt.rssi = (int8_t)radio.testCarrier() ? -50 : -100;
            
            Serial.println("✓ Captured " + String(len) + " bytes");
            for (uint8_t b : pkt.data) {
                Serial.print(String(b, 16) + " ");
            }
            Serial.println();
            return pkt;
        }
        delay(10);
    }
    
    Serial.println("✗ No packet captured");
    return pkt;
}

ReplayResult replayPacket(const PacketCapture &packet, uint8_t repeatCount) {
    ReplayResult result{false, 0, 0};
    
    Serial.println("\n=== NRF24 Packet Replay ===");
    Serial.println("Channel: " + String(packet.channel));
    Serial.println("Packet size: " + String(packet.data.size()) + " bytes");
    Serial.println("Repeat: " + String(repeatCount) + "x");
    
    if (!TxArm::isArmed()) {
        Serial.println("✗ TX not armed (hold BACK button)");
        return result;
    }
    
    radio.begin();
    radio.setAutoAck(false);
    radio.setRetries(0, 0);
    radio.disableCRC();
    radio.setChannel(packet.channel);
    radio.setPALevel(RF24_PA_MAX);
    radio.stopListening();
    
    for (uint8_t i = 0; i < repeatCount; i++) {
        if (radio.write(packet.data.data(), packet.data.size())) {
            result.packetsSent++;
        }
        result.replayCount++;
        delay(100);
    }
    
    result.success = (result.packetsSent > 0);
    
    Serial.println("✓ Replayed " + String(result.packetsSent) + "/" + String(result.replayCount) + " packets");
    
    return result;
}

}  // namespace Nrf24Replay
