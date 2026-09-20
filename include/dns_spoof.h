#pragma once
#include <Arduino.h>
namespace DNSSpoof {
struct SpoofResult { bool active; String targetDomain; String spoofedIP; uint32_t requestsIntercepted; };
SpoofResult start(const String &domain, const String &spoofIP = "192.168.1.1", uint16_t timeoutMs = 60000);
void stop();
uint32_t getInterceptedRequests();
} // namespace DNSSpoof
