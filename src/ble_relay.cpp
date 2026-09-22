#include "ble_relay.h"
namespace BLERelay {
RelayResult startRelay(const String &targetMAC, uint16_t timeoutMs) {
    return {false, targetMAC, 0, 0};
}
void stop() {}
uint16_t estimateRange() { return 0; }
} // namespace BLERelay
