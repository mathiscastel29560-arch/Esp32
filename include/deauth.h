#pragma once
#include <Arduino.h>

namespace Deauth {

void send(const String &bssid, const String &clientMac, uint8_t channel, int count);

} // namespace Deauth
