#include "core/powerSave.h"
#include <interface.h>

/***************************************************************************************
** Function name: _setup_gpio()
** Location: main.cpp
** Description:   initial setup for the device
***************************************************************************************/
void _setup_gpio() {
    pinMode(UP_BTN, INPUT_PULLUP);
    pinMode(DW_BTN, INPUT_PULLUP);
    pinMode(SEL_BTN, INPUT_PULLUP);
    pinMode(ESC_BTN, INPUT_PULLUP);

    pinMode(CC1101_SS_PIN, OUTPUT);
    digitalWrite(CC1101_SS_PIN, HIGH);
    pinMode(NRF24_SS_PIN, OUTPUT);
    digitalWrite(NRF24_SS_PIN, HIGH);
    pinMode(TFT_CS, OUTPUT);
    digitalWrite(TFT_CS, HIGH);

    // This board has one CC1101 and one NRF24 module wired, not the M5
    // one-pin-radio modules Bruce defaults to.
    bruceConfigPins.rfModule = CC1101_SPI_MODULE;
}

/*********************************************************************
** Function: setBrightness
** location: settings.cpp
** set brightness value
**********************************************************************/
void _setBrightness(uint8_t brightval) {
    if (brightval == 0) {
        analogWrite(TFT_BL, brightval);
    } else {
        int bl = MINBRIGHT + round(((255 - MINBRIGHT) * brightval / 100));
        analogWrite(TFT_BL, bl);
    }
}

/*********************************************************************
** Function: InputHandler
** Handles the variables PrevPress, NextPress, SelPress, AnyKeyPress and EscPress
** Only 4 physical buttons on this board (no L/R): UP/DOWN double as
** Prev/Next for list navigation, matching how the audit firmware's own
** menu.cpp treats the same 4 GPIOs.
**********************************************************************/
void InputHandler(void) {
    static unsigned long tm = 0;
    if (millis() - tm < 200 && !LongPress) return;

    bool _u = digitalRead(UP_BTN);
    bool _d = digitalRead(DW_BTN);
    bool _s = digitalRead(SEL_BTN);
    bool _e = digitalRead(ESC_BTN);

    if (!_u || !_d || !_s || !_e) {
        tm = millis();
        if (!wakeUpScreen()) AnyKeyPress = true;
        else return;
    }
    if (!_u) {
        UpPress = true;
        PrevPress = true;
        PrevPagePress = true;
    }
    if (!_d) {
        DownPress = true;
        NextPress = true;
        NextPagePress = true;
    }
    if (!_s) { SelPress = true; }
    if (!_e) { EscPress = true; }
}

/*********************************************************************
** Function: powerOff
** location: mykeyboard.cpp
** Turns off the device (or try to)
** No PMIC on this board to command a shutdown — the physical slide
** switch cuts power directly (see HARDWARE.md); nothing for firmware
** to do here.
**********************************************************************/
void powerOff() {}

/*********************************************************************
** Function: checkReboot
** location: mykeyboard.cpp
** Btn logic to turn off the device (name is odd btw)
** No PMIC-driven power-off available (see powerOff() above), so this
** is a no-op rather than a fake countdown UI that can't act on it.
**********************************************************************/
void checkReboot() {}
