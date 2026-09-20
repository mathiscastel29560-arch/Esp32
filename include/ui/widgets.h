#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>

// Pure drawing primitives, reused by every screen in ui.h. Each one draws
// into whatever sprite you hand it (usually the screen's own canvas) and
// touches nothing else — no state, no input polling, no module calls.
// Every color/spacing value comes from ui/theme.h.
namespace Widgets {

// One selectable row: fills its own background (selected = surface tone +
// accent outline, matching the main menu's highlight), draws the label,
// and an optional RSSI bar / badge chip on the right if provided.
void listRow(TFT_eSprite &c, int16_t x, int16_t y, int16_t w, int16_t h,
             const String &label, bool selected,
             bool hasRssi = false, int rssi = 0, const String &badge = "");

// "Label: value" line for detail screens — label dim, value bright.
void labelValue(TFT_eSprite &c, int16_t x, int16_t y, int16_t w, const String &label,
                const String &value);

// Signal-strength bar, gradient red (weak) -> amber -> green (strong).
// rssi is dBm (typically -100..-30); out-of-range values clamp.
void rssiBar(TFT_eSprite &c, int16_t x, int16_t y, int16_t w, int16_t h, int rssi);

// Small rounded chip with text, e.g. a manufacturer name or "random MAC".
void badge(TFT_eSprite &c, int16_t x, int16_t y, const String &text, uint16_t color);

// Rotating-arc spinner for "scan in progress" — pass millis() each call.
void spinner(TFT_eSprite &c, int16_t cx, int16_t cy, int16_t radius, uint32_t nowMs);

// Determinate progress bar, fraction clamped to 0..1.
void progressBar(TFT_eSprite &c, int16_t x, int16_t y, int16_t w, int16_t h, float fraction);

} // namespace Widgets
