#include "buzzer.h"
#include "config.h"

namespace Buzzer {

void begin() {
    pinMode(PIN_BUZZER, OUTPUT);
    noTone(PIN_BUZZER);
}

void beep(uint16_t freqHz, uint16_t durationMs) {
    tone(PIN_BUZZER, freqHz, durationMs);
}

void chirpOk() {
    beep(2600, 60);
    delay(90);
    beep(2600, 60);
}

void chirpAlert() {
    beep(1200, 250);
}

} // namespace Buzzer
