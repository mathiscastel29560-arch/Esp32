#include "nrf24_injection.h"
#include "config.h"
#include "tx_arm.h"
#include <RF24.h>

namespace {
RF24 radio(PIN_NRF24_CE, PIN_NRF24_CS);
}

namespace Nrf24Injection {

InjectionResult injectPacket(uint8_t channel, const std::vector<uint8_t> &payload, uint8_t repeatCount) {
    InjectionResult result{false, 0, channel};
    
    Serial.println("\n=== NRF24 Packet Injection ===");
    Serial.println("Channel: " + String(channel));
    Serial.println("Payload: " + String(payload.size()) + " bytes");
    Serial.println("Repeats: " + String(repeatCount));
    
    if (!TxArm::isArmed()) {
        Serial.println("✗ TX not armed");
        return result;
    }
    
    radio.begin();
    radio.setAutoAck(false);
    radio.setChannel(channel);
    radio.setPALevel(RF24_PA_MAX);
    radio.stopListening();
    
    for (uint8_t i = 0; i < repeatCount; i++) {
        if (radio.write(payload.data(), payload.size())) {
            result.packetsInjected++;
        }
        delay(50);
    }
    
    result.success = (result.packetsInjected > 0);
    Serial.println("✓ Injected " + String(result.packetsInjected) + " packets");
    
    return result;
}

}  // namespace Nrf24Injection
