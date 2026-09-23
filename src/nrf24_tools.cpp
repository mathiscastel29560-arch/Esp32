#include "nrf24_tools.h"
#include "config.h"
#include <RF24.h>

namespace {
RF24 radio(PIN_NRF24_CE, PIN_NRF24_CS);
}

namespace Nrf24Tools {

void begin() {
    radio.begin();
    radio.setAutoAck(false);
    radio.setRetries(0, 0);
    radio.disableCRC();
    radio.setAddressWidth(3);
    radio.setPALevel(RF24_PA_MIN);
    radio.startListening();
}

std::vector<uint8_t> scanChannels(uint16_t samplesPerChannel) {
    std::vector<uint8_t> activity(126, 0);
    for (uint8_t ch = 0; ch < 126; ch++) {
        radio.setChannel(ch);
        uint16_t hits = 0;
        for (uint16_t s = 0; s < samplesPerChannel; s++) {
            radio.startListening();
            delayMicroseconds(128);
            radio.stopListening();
            if (radio.testCarrier()) hits++;
        }
        activity[ch] = (uint8_t)min((uint16_t)255, hits);
    }
    return activity;
}

} // namespace Nrf24Tools
