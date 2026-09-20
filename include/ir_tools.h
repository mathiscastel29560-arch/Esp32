#pragma once
#include <Arduino.h>
#include <vector>

// IR transmit/receive — a universal remote, functionally: no different
// from any other IR blaster/learning remote. Not TX-arm gated (unlike the
// RF features), since toggling a TV/display over line-of-sight IR has
// nothing like their range or third-party impact.
namespace IrTools {

void begin();

// Best-effort: cycles through a small built-in list of commonly-published
// TV power-toggle codes (a handful of brands/protocols). This is a "spray
// and pray" approach like the classic TV-B-Gone — it won't hit every model,
// since there's no single universal code. For a specific TV, use learn()
// on its actual remote instead, which is reliable.
void sendUniversalPowerToggle();

struct IrCapture {
    std::vector<uint16_t> rawUs; // raw mark/space durations, like SubGhz::Capture
};

// Waits up to timeoutMs for one IR remote button press and captures its
// raw timing. Empty result if nothing was seen.
IrCapture learn(uint32_t timeoutMs = 5000);

// Re-transmits a previously learned capture.
void replay(const IrCapture &capture);

} // namespace IrTools
