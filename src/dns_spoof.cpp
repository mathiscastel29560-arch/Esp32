#include "dns_spoof.h"
namespace DNSSpoof {
SpoofResult start(const String &domain, const String &spoofIP, uint16_t timeoutMs) {
    return {false, domain, spoofIP, 0};
}
void stop() {}
uint32_t getInterceptedRequests() { return 0; }
} // namespace DNSSpoof
