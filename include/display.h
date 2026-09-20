#pragma once
#include <Arduino.h>

class TFT_eSPI;

// Owns the physical TFT_eSPI instance and draws a transient plain-text
// "booting..." placeholder at power-on, before ui/ui.h's sprite-based
// splash/home/list/detail screens (see ui/ui.h) take over everything else.
namespace Display {

void begin();

// The underlying TFT_eSPI object, for ui/ui.cpp to build its sprite on top
// of instead of creating a second instance bound to the same pins.
TFT_eSPI &raw();

} // namespace Display
