# Premium UI/UX Design System

## Overview

This ESP32 audit tool features a **premium dark-mode UI** designed for the TFT ILI9341 320x240 color display. The design emphasizes:

- **Elegant simplicity**: Minimal visual clutter, clear hierarchy
- **Premium dark theme**: Sophisticated dark background with subtle elevation
- **Consistent semantics**: Color-coded status (green=good, amber=fair, red=critical)
- **Smooth animations**: 60fps-capable transitions and interactions
- **Reusable components**: Centralized tokens and modular widgets

### Design Philosophy

- **Tokens over hard-coded values**: Every color, spacing, and timing lives in `theme.h`. Change the whole look by editing one file.
- **PSRAM-backed rendering**: Off-screen sprite prevents flicker; finished frames push atomically.
- **Depth via surfaces**: Dark background + subtle surface elevation creates visual hierarchy without heavy shadows.
- **Semantic colors**: RSSI bars, status indicators, alerts all use consistent red→amber→green gradients.

---

## Color Palette (RGB565)

| Name | Hex | Usage |
|------|-----|-------|
| **BG** | #080810 | Screen background (almost black) |
| **SURFACE** | #0F1218 | Elevated surfaces (list items, cards) |
| **SURFACE_ALT** | #182151 | Depth variation (status bar) |
| **TEXT** | #F8F8FF | Primary text (off-white) |
| **TEXT_DIM** | #7B7B8F | Secondary/disabled text |
| **ACCENT** | #1DD4BF | Primary action (buttons, highlights) |
| **ACCENT_LIGHT** | #2FFFFF | Light variant (hover states) |
| **OK** | #2BD420 | Success (green) |
| **WARN** | #FDAA00 | Warning (amber) |
| **DANGER** | #FF4008 | Critical (red) |
| **INFO** | #5580FF | Informational (blue) |

---

## Spacing & Metrics (8px Grid)

```cpp
SPACE_1: 8px    // Tight margins (within elements)
SPACE_2: 16px   // Standard spacing (section dividers)
SPACE_3: 24px   // Generous spacing (major sections)
SPACE_4: 32px   // Large separation (screen-level)

RADIUS_SM: 4px  // Small rounded corners (buttons, pills)
RADIUS_MD: 8px  // Medium (list items, cards)
RADIUS_LG: 12px // Large (major containers)

STATUS_BAR_H: 32px
ROW_H: 40px     // List row height (spacious)
```

---

## Typography

Three font families (smooth/anti-aliased):

- **FONT_TITLE** (NotoSansBold36): Large headers, splash screen
- **FONT_BODY** (NotoSansBold15): List items, labels, status text
- **FONT_MONO** (NotoSansMonoSCB20): IP addresses, technical values

**Text Colors:**
- White/off-white for primary text (bright, selected state)
- Dim gray for secondary/disabled text
- Semantic colors (green/amber/red) for status

---

## Animation Timings

```cpp
ANIM_SELECTION_MS: 140ms  // List highlight slide (ease-out quad)
ANIM_TRANSITION_MS: 260ms // Screen-to-screen transition
ANIM_PULSE_MS: 400ms      // Gentle loading/idle pulse
SPLASH_DURATION_MS: 5000ms
```

All animations use **ease-out quad** for a premium, natural feel.

---

## Core Components

### 1. Status Bar (`drawStatusBar`)

Displays at the top of every screen:
- **Left**: Current time (HH:MM)
- **Right**: Battery % (color-coded), GPS fix indicator, radio activity indicator

```cpp
Ui::StatusInfo status;
status.time = "14:32:45";
status.battPercent = 85;
status.gpsFix = true;
status.radioActive = false;
Ui::showList(status, "WiFi Networks", items, selectedIndex);
```

### 2. List Screen (`showList`)

Scrollable list with:
- **Selection highlight**: Animated capsule that slides between rows
- **RSSI bars**: Semantic color gradient (red→amber→green) for signal strength
- **Badges**: Colored chips for attributes (e.g., "Active", manufacturer)
- **Text clipping**: Ellipsis if label exceeds width

```cpp
std::vector<Ui::ListItem> items;
items.push_back({"WiFi Network Alpha", true, -45, "Active"});
items.push_back({"BLE Device Beta", true, -65, ""});
Ui::showList(status, "Scan Results", items, selectedIdx);
```

### 3. Detail Screen (`showDetail`)

Key-value pairs with:
- **Labels** (dim text on left)
- **Values** (bright text on right)
- **Badges** (colored chips for attributes)

```cpp
std::vector<Ui::DetailRow> rows = {
    {"SSID", "WiFi-Alpha"},
    {"Channel", "6"},
    {"Encryption", "WPA2"}
};
std::vector<Ui::Badge> badges = {
    {"5GHz", Theme::COLOR_INFO},
    {"AES", Theme::COLOR_OK}
};
Ui::showDetail(status, "Network Details", rows, badges);
```

### 4. Widgets

All widgets are **pure drawing functions** — no state, no side effects:

#### `listRow(sprite, x, y, w, h, label, selected, hasRssi, rssi, badge)`
Single row in a list: text + optional RSSI bar + optional badge.

#### `rssiBar(sprite, x, y, w, h, rssi)`
Horizontal signal-strength bar with semantic gradient.
- Input: `rssi` in dBm (typically -100 to -30)
- Output: Bar width = strength, color = red→amber→green

#### `badge(sprite, x, y, text, color)`
Rounded pill with colored border and faint background.

#### `labelValue(sprite, x, y, w, label, value)`
"Label: value" line (label dim, value bright).

#### `spinner(sprite, cx, cy, radius, nowMs)`
Rotating arc indicator for "scan in progress".

#### `progressBar(sprite, x, y, w, h, fraction)`
Determinate progress bar (0.0–1.0 fill).

---

## Screen Layout Template

Every screen follows this structure:

```
┌─────────────────────────────────┐
│  Status Bar (32px)              │  ← Time, battery, GPS, radio
├─────────────────────────────────┤
│  Content Area (header)          │
│  ─                              │
│  [List / Detail / Graph / ...]  │
│  [List / Detail / Graph / ...]  │
│  [List / Detail / Graph / ...]  │
│                                 │
├─────────────────────────────────┤
│  Help Text (if any)             │  ← "OK: confirm  BACK: exit"
└─────────────────────────────────┘
```

### Margins & Padding

- **Screen edges**: `SPACE_2` (16px) margin on all sides
- **Between sections**: `SPACE_3` (24px) gap
- **Within elements**: `SPACE_1` (8px) padding

---

## Rendering Pipeline (No Flicker)

1. **Canvas allocation**: `TFT_eSprite` allocated in PSRAM
2. **Draw everything into sprite**: All text, shapes, widgets drawn off-screen
3. **Atomic push**: Entire frame pushed to TFT in one call
4. **Frame throttling**: Max ~30fps to prevent starvation of WiFi/BLE tasks

```cpp
// In ui.cpp's showList():
canvas.fillSprite(Theme::COLOR_BG);       // Fill
drawStatusBar(status);                    // Draw status
// ... draw list items, etc. ...
presentFrame(screenChanged);              // Push + optional transition
```

### Screen Transitions

When `screenChanged = true`:
- **Slide animation**: Previous frame slides left while new frame enters from right
- **Duration**: `ANIM_TRANSITION_MS` (260ms, ease-out quad)
- **Fallback**: If transition interrupted, jump to final frame

---

## Implementing a Custom Screen

1. **Allocate sprite** (or reuse canvas from existing screen):
   ```cpp
   TFT_eSprite sprite(&tft);
   sprite.setColorDepth(16);
   sprite.createSprite(width, height);  // PSRAM-backed
   ```

2. **Draw background**:
   ```cpp
   sprite.fillSprite(Theme::COLOR_BG);
   drawStatusBar(status);
   ```

3. **Draw content** (use widgets, theme tokens only):
   ```cpp
   int y = Theme::STATUS_BAR_H + Theme::SPACE_2;
   sprite.loadFont(FONT_BODY);
   sprite.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
   sprite.drawString("My Title", Theme::SPACE_2, y);
   // ... more drawing ...
   sprite.unloadFont();
   ```

4. **Push frame**:
   ```cpp
   sprite.pushSprite(0, 0);
   ```

---

## Future Widgets (Not Yet Implemented)

These would follow the same stateless, token-based pattern:

- **Waterfall graph**: Time-series visualization (frequency spectrogram)
- **Histogram**: Vertical bars for frequency/channel distribution
- **Heatmap**: 2D intensity grid (e.g., RF power across time/freq)
- **Gauge**: Circular progress indicator for signal quality
- **Indicator** (pulsing dot): For "scanning" or "transmitting" states

Each would take data and draw into a given sprite, nothing more.

---

## Testing the Design

Run the **Style Guide screen** to validate:
- Color palette
- Typography hierarchy
- Widget rendering (badges, RSSI bars, list items)
- Spacing grid
- Sample list screen

**To invoke** (from menu or main.cpp):
```cpp
#include "ui/style_guide.h"
StyleGuide::show();  // Blocks, shows design validation screens
```

---

## Token Reference

All tokens centralized in `include/ui/theme.h`:

```cpp
namespace Theme {
  // Colors (RGB565)
  constexpr uint16_t COLOR_BG;
  constexpr uint16_t COLOR_SURFACE;
  // ... etc ...
  
  // Spacing (pixels)
  constexpr uint8_t SPACE_1, SPACE_2, SPACE_3, SPACE_4;
  constexpr uint8_t RADIUS_SM, RADIUS_MD, RADIUS_LG;
  
  // Sizing
  constexpr uint8_t STATUS_BAR_H, ROW_H;
  
  // Animation (milliseconds)
  constexpr uint16_t ANIM_SELECTION_MS, ANIM_TRANSITION_MS, ANIM_PULSE_MS;
}
```

**Edit this file to change the entire look.**

---

## Responsive Considerations

- **TFT (320x240)**: Primary target; full-featured color UI
- **OLED (128x64)**: Fallback monochrome; simplified layouts, no colors
  - OLED impl in `include/ui/oled_ui.h` and `src/ui/oled_ui.cpp`
  - Detectable via `Display::kind()`

All color-based widgets gracefully degrade on OLED (monochrome only).

---

## Performance Tips

- **Throttle redraws**: Don't call `showList()` faster than ~30fps
- **Use static strings**: Avoid allocating large strings in tight loops
- **Batch font loading**: Load font once, draw multiple items, unload
- **Minimal transitions**: Only transition on screen *identity* change, not content update

---

## Files

- `include/ui/theme.h` — Design tokens (colors, spacing, timings)
- `include/ui/ui.h` — Public screen API
- `include/ui/widgets.h` — Reusable drawing primitives
- `src/ui/ui.cpp` — Screen implementations
- `src/ui/widgets.cpp` — Widget implementations
- `include/ui/style_guide.h` — Design validation tool
- `src/ui/style_guide.cpp` — Style guide implementation
- `include/ui/oled_ui.h` — OLED fallback
- `src/ui/oled_ui.cpp` — OLED implementations

---

## Changelog

### v1.0 (Premium Dark Mode)
- Introduced dark theme with teal accent
- Expanded spacing (ROW_H: 32→40, STATUS_BAR_H: 28→32)
- Improved RSSI bar rendering with subtle borders
- Enhanced status bar with better indicator placement
- Added COLOR_SURFACE_ALT for depth variation
- Extended animation timings for smoother feel (120→140ms, 220→260ms)
- Created style guide for design validation
