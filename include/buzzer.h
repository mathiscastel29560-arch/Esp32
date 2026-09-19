#pragma once
#include <Arduino.h>

namespace Buzzer {

void begin();
void beep(uint16_t freqHz = 2000, uint16_t durationMs = 80);
void chirpOk();       // short double beep: action succeeded / fix acquired
void chirpAlert();    // longer beep: TX action fired / GPS lost / error

} // namespace Buzzer
