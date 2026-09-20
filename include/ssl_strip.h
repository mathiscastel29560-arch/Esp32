#pragma once
#include <Arduino.h>
namespace SSLStrip {
struct StripResult { bool active; uint32_t httpsDowngraded; uint32_t dataLogged; };
StripResult startStripping(uint16_t timeoutMs = 60000);
void stop();
String getLoggedData();
} // namespace SSLStrip
