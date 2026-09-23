#include "buttons.h"
#include "config.h"

namespace {
struct BtnState {
    uint8_t pin;
    int lastRead;
    uint32_t lastChange;
};

BtnState btns[4] = {
    {PIN_BTN_UP, HIGH, 0},
    {PIN_BTN_DOWN, HIGH, 0},
    {PIN_BTN_SELECT, HIGH, 0},
    {PIN_BTN_BACK, HIGH, 0},
};

constexpr uint32_t DEBOUNCE_MS = 40;
} // namespace

namespace Buttons {

void begin() {
    for (auto &b : btns) pinMode(b.pin, INPUT_PULLUP);
}

Button poll() {
    uint32_t now = millis();
    for (int i = 0; i < 4; i++) {
        int reading = digitalRead(btns[i].pin);
        if (reading != btns[i].lastRead && now - btns[i].lastChange > DEBOUNCE_MS) {
            btns[i].lastChange = now;
            btns[i].lastRead = reading;
            if (reading == LOW) { // press edge (active low, pulled up)
                return (Button)(i + 1); // matches UP,DOWN,SELECT,BACK order above
            }
        }
    }
    return NONE;
}

bool isHeld(Button b) {
    if (b == NONE) return false;
    return digitalRead(btns[b - 1].pin) == LOW;
}

} // namespace Buttons
