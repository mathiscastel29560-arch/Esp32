#pragma once
#include <Arduino.h>

// Little pixel-art shark mascot for the OLED — shown on the boot splash and
// as an occasional "swim-by" on the idle status screen. Purely cosmetic,
// no functional role.
namespace Mascot {

extern const uint8_t SHARK_BITMAP[] PROGMEM;
constexpr uint8_t WIDTH = 32;
constexpr uint8_t HEIGHT = 16;

} // namespace Mascot
