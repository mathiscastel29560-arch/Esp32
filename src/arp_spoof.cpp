#include "arp_spoof.h"
namespace ARPSpoof {
SpoofResult startMITM(const String &targetIP, const String &gatewayIP, uint16_t timeoutMs) {
    return {false, targetIP, gatewayIP, 0};
}
void stop() {}
} // namespace ARPSpoof
