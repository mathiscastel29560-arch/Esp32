#include "ble_jamming.h"
namespace BLEJamming {
JamResult startJamming(uint32_t durationMs, uint8_t powerLevel) { return {false, 0, 0, "BLE jamming - legal/ethical constraints apply"}; }
void stopJamming() {}
bool isJamming() { return false; }
} // namespace BLEJamming
