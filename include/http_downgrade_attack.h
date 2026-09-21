#pragma once
#include <Arduino.h>

namespace HTTPDowngradeAttack {

struct DowngradeResult {
    bool success;
    uint32_t redirectsCount;
    uint32_t credentialsIntercepted;
    uint32_t durationMs;
};

// HTTP downgrade attack (SSL Strip simulation)
// Intercepts HTTPS and downgrades to HTTP for credential harvesting
DowngradeResult executeDowngrade(uint32_t durationMs = 30000);

void stop();
bool isActive();

}  // namespace HTTPDowngradeAttack
