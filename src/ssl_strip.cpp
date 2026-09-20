#include "ssl_strip.h"
namespace SSLStrip {
StripResult startStripping(uint16_t timeoutMs) { return {false, 0, 0}; }
void stop() {}
String getLoggedData() { return "No data logged"; }
} // namespace SSLStrip
