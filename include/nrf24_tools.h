#pragma once
#include <Arduino.h>
#include <vector>

// Passive 2.4GHz channel-activity scanner (spectrum-analyzer style) using
// the NRF24L01's carrier-detect feature. Receive-only, nothing is sent.
namespace Nrf24Tools {

void begin();

// One entry per RF channel (0-125, i.e. 2400-2525MHz), each a 0-255 count
// of how often a carrier was detected while listening on that channel —
// higher means busier. Useful for spotting Wi-Fi/BLE/2.4GHz-ISM congestion
// around the target site.
std::vector<uint8_t> scanChannels(uint16_t samplesPerChannel = 50);

} // namespace Nrf24Tools
