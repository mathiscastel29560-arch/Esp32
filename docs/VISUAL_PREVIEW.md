# Premium UI - Visual Preview

This document shows how the redesigned UI looks on the TFT ILI9341 320x240 display.

---

## Color Palette

```
┌─────────────────────────────────────────────────────┐
│  BACKGROUND                                         │
│  #080810 - Almost black with blue undertone        │
│  Used for: Screen backgrounds, large empty spaces  │
└─────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────┐
│  SURFACE                          SURFACE_ALT       │
│  #0F1218                          #182151            │
│  Elevated containers              Depth variation   │
│  List items, cards                Status bars      │
└─────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────┐
│  TEXT                             TEXT_DIM          │
│  #F8F8FF (Off-white)              #7B7B8F (Gray)   │
│  Primary content                  Secondary text   │
│  Bright, active, selected         Disabled, hints  │
└─────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────┐
│  ACCENT                           ACCENT_LIGHT     │
│  #1DD4BF (Vibrant cyan-teal)      #2FFFFF          │
│  Primary actions                  Hover/disabled   │
│  Highlights, buttons              Faded variant    │
└─────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────┐
│  SEMANTIC COLORS                                    │
│  ● OK:     #2BD420 (Vibrant green)   - Good/Strong │
│  ● WARN:   #FDAA00 (Amber/orange)    - Fair/Caution│
│  ● DANGER: #FF4008 (Bold red-orange) - Bad/Critical│
│  ● INFO:   #5580FF (Blue)            - Information │
└─────────────────────────────────────────────────────┘
```

---

## Status Bar (32px Height)

```
╔════════════════════════════════════════════════════╗
║ 14:32                                         85% ●●║  ← Time, Battery %, GPS fix ●, Radio ●
╚════════════════════════════════════════════════════╝
│ Subtle border below (SURFACE_ALT color)            │
└────────────────────────────────────────────────────┘
```

**Elements:**
- **Left**: Current time (HH:MM format, FONT_BODY, white text)
- **Right**: Battery percentage (color-coded: green if >50%, amber 20-50%, red <20%)
- **Indicators**: 
  - Green dot if GPS has fix
  - Red dot if radio is actively transmitting

---

## List Screen - WiFi Scan Results

```
╔════════════════════════════════════════════════════╗
║ 14:32                                         85% ●║  ← Status Bar (SURFACE)
╚════════════════════════════════════════════════════╝
                                                    ◎  ← Spinner (if scanning)
┌────────────────────────────────────────────────────┐
│ WiFi NETWORKS                                      │  ← Title (FONT_BODY, TEXT)
├────────────────────────────────────────────────────┤
│ ┌────────────────────────────────────────────────┐ │
│ │ ► WiFi-Alpha                    ▬▬▬ Active    │ │  ← Selected row (animated)
│ │   SURFACE bg + ACCENT border, text brighter   │ │
│ └────────────────────────────────────────────────┘ │
│                                                    │
│    iPhone 12 (BLE)    ━━━━━░░ [BLE]             │  ← Next row (not selected)
│    (TEXT_DIM, faded)                            │     RSSI bar + badge visible
│                                                    │
│    ZigBee-Node-7      ━░░░░░░░ (no badge)      │
│                                                    │
│                                                    │
│                 ▲/▼: navigate  ●: select  ◄: back │  ← Help text (TEXT_DIM, small)
└────────────────────────────────────────────────────┘
```

**Features:**
- **Row Height**: 40px (spacious)
- **Selection Capsule**: Animated slide between rows
  - Filled: SURFACE (elevated)
  - Border: ACCENT (2px, rounded)
- **RSSI Bar**: 
  - Background: SURFACE_ALT with TEXT_DIM border
  - Fill: Semantic gradient (red → amber → green)
  - Shows signal strength visually
- **Badge**: Optional colored chip (e.g., "Active", "BLE")
  - Border + very faint background
  - Fixed width per label

---

## Detail Screen - WiFi Network Properties

```
╔════════════════════════════════════════════════════╗
║ 14:32                                         85% ●║  ← Status Bar
╚════════════════════════════════════════════════════╝

    NETWORK DETAILS                                    ← Title (FONT_BODY, ACCENT)

    ● 5GHz    ● AES    ● Strong                      ← Badges (3 colors)

    SSID:          WiFi-Alpha                         ← Labels (TEXT_DIM) : Values (TEXT)
    Channel:       6
    Encryption:    WPA2-PSK/AES
    Signal:        -45 dBm
    Clients:       3
    Security:      WPA2 Only
    Last Seen:     2024-09-21 14:32:10


    Press OK for more info...                        ← Help text (TEXT_DIM)
```

**Features:**
- **Title**: FONT_BODY, ACCENT color
- **Badges**: Multiple attribute pills (color-coded)
  - Very faint background + border in badge color
  - Wrapped left-to-right
- **Detail Rows**:
  - Label (left): TEXT_DIM, aligned left
  - Value (right): TEXT, aligned right
  - 16px vertical spacing
- **Help**: Bottom line, TEXT_DIM, center

---

## Typography Scale

```
╔════════════════════════════════════════════════════╗
║ 14:32                                              ║
╚════════════════════════════════════════════════════╝

        TITLE FONT (NotoSansBold36)
        Used for: Splash screen, major headers
        ~36px size, tight line-height

    BODY FONT (NotoSansBold15)
    Used for: Menu items, labels, status text, badges
    ~15px size, readable on 320x240

    DIM TEXT (same FONT_BODY, different color)
    Used for: Secondary info, disabled items, hints
    Same size, softer appearance

    192.168.1.1  (FONT_MONO)
    Used for: IP addresses, MAC addresses, technical values
    Monospaced, consistent width
```

---

## Spacing Grid (8px Base)

```
SPACE_1: 8px   ■ Tight margins within elements
SPACE_2: 16px  ■■ Standard spacing (section dividers)
SPACE_3: 24px  ■■■ Generous spacing (major sections)
SPACE_4: 32px  ■■■■ Large separation (screen-level)

RADIUS_SM: 4px    ■ Small corners (pills, small buttons)
RADIUS_MD: 8px    ■■ Medium corners (list items, cards)
RADIUS_LG: 12px   ■■■ Large corners (major containers)

Example Layout:
┌─────────────────────────────────────────┐
│  ↔ SPACE_2                              │
│ ┌───────────────────────────────────┐   │
│ │ ↔ SPACE_1                         │   │
│ │ Content Area                      │   │
│ │ ↔ SPACE_1                         │   │  Spacing around edges
│ └───────────────────────────────────┘   │  and between sections
│                                          │
│  ↔ SPACE_3 (gap to next section)        │
│                                          │
│ ┌───────────────────────────────────┐   │
│ │ More Content                      │   │
│ └───────────────────────────────────┘   │
│  ↔ SPACE_2                              │
└─────────────────────────────────────────┘
```

---

## RSSI Bar - Signal Strength Visualization

```
Weak Signal (-95 dBm):
  ┌────────────┐
  │░░░░░░░░░░░│  (Red fill, ~10% of bar)
  └────────────┘

Fair Signal (-65 dBm):
  ┌────────────┐
  │▓▓▓▓▓░░░░░░│  (Amber fill, ~50% of bar)
  └────────────┘

Good Signal (-45 dBm):
  ┌────────────┐
  │▓▓▓▓▓▓▓░░░░│  (Green fill, ~80% of bar)
  └────────────┘

Strong Signal (-30 dBm):
  ┌────────────┐
  │▓▓▓▓▓▓▓▓▓▓▓│  (Bright green, 100% full)
  └────────────┘

Features:
- Background: SURFACE_ALT with subtle border (TEXT_DIM)
- Fill: Gradient color (semantic: RED → AMBER → GREEN)
- Height: Proportional to visible space (typically 10-12px)
- Smooth rendering: No aliasing on rounded corners
```

---

## Animation Timeline

```
Selection Highlight Slide (140ms ease-out):
  Frame 0:     Row 0 selected  ← Highlight at row 0
  ...
  Frame N:     Row 2 selected  ← Smoothly slides to row 2
  
  Duration: 140ms, ease-out quad (starts fast, ends slow)

Screen Transition (260ms ease-out):
  Frame 0:     Previous screen visible → New screen slides in from right
  ...
  Frame N:     New screen fully visible, previous hidden
  
  Duration: 260ms, ease-out quad

Loading Pulse (400ms, repeating):
  Frame 0:     Normal opacity (alpha = 1.0)
  ...
  Frame N:     Slightly dimmed (alpha = 0.7)
  
  Duration: 400ms per cycle, ease-in-out
```

---

## Screen Behavior Examples

### Scenario 1: User Navigates Menu

```
Time 0ms:
┌──────────────────────┐
│ ▶ WiFi Tools         │  ← Selected
│   BLE Tools          │
│   RF/2.4GHz          │
└──────────────────────┘

User presses DOWN

Time 0-140ms (Animation playing):
┌──────────────────────┐
│   WiFi Tools         │
│ ▶ BLE Tools          │  ← Highlight sliding down (ease-out)
│   RF/2.4GHz          │
└──────────────────────┘

Time 140ms+:
┌──────────────────────┐
│   WiFi Tools         │
│ ▶ BLE Tools          │  ← Selection complete
│   RF/2.4GHz          │
└──────────────────────┘
```

### Scenario 2: Screen Change

```
Time 0ms:
┌──────────────────────┐
│ MAIN MENU            │
│ ▶ WiFi Tools         │  ← Currently visible
│   BLE Tools          │
└──────────────────────┘

User presses OK → Screen change triggered

Time 0-260ms (Transition animating):
┌──────────────────────┐  ┌──────────────────────┐
│ ████████░░░░░░░░░░░░│  │░░░░░░░░░░░████████   │
│ MAIN MENU (sliding ←)│  │ WiFi NETWORKS (→ in) │
└──────────────────────┘  └──────────────────────┘
            ^                        ^
       Previous screen          New screen
       (fading out left)        (sliding in right)

Time 260ms+:
┌──────────────────────┐
│ WiFi NETWORKS        │  ← New screen fully visible
│ ▶ WiFi-Alpha    [■] │
│   iPhone 12         │
└──────────────────────┘
```

---

## Comparison: Before vs After

### BEFORE (Original)
- Minimal dark background (#0D0D0F)
- Flatter appearance, less elevation
- Smaller spacing (ROW_H: 32px, STATUS_BAR_H: 28px)
- Faster animations (140ms/220ms) → felt jerky
- Basic RSSI rendering
- No style guide tool

### AFTER (Premium Redesign) ✨
- Sophisticated dark with elevation (#080810 + #0F1218)
- Clear visual hierarchy with surface depth
- Generous spacing (ROW_H: 40px, STATUS_BAR_H: 32px)
- Smooth animations (140ms/260ms) → premium feel
- Refined RSSI with border + gradient
- Interactive style guide for validation

---

## How to Validate on Device

### Method 1: Run Style Guide
```
1. Add to your test/main code:
   #include "ui/style_guide.h"
   StyleGuide::show();

2. Watch 5 screens:
   - Color palette (all 11 tokens)
   - Typography (all fonts)
   - Widgets (RSSI, badges, list)
   - Spacing grid (SPACE_1..4)
   - Sample list (full example)

3. Press any button to advance
```

### Method 2: Live on Real Menus
```
1. Flash firmware to ESP32
2. Use 4-button interface to navigate
3. Observe:
   - Smooth selection highlight slide
   - Screen transition animation
   - Color consistency across buttons
   - RSSI bar gradients
```

### Method 3: Code Inspection
- `include/ui/theme.h` - All tokens defined
- `src/ui/ui.cpp` - Screen rendering code
- `src/ui/widgets.cpp` - Widget drawing code
- `docs/UI_DESIGN.md` - Complete reference

---

## Performance Notes

- **Rendering**: No flicker (PSRAM sprite buffer)
- **Frame rate**: Capped at ~30fps (doesn't starve WiFi/BLE)
- **Animation CPU**: Minimal (simple math, no heavy transformations)
- **Memory**: Status bar / widgets reuse theme tokens (no dynamic allocation)

---

## Accessibility

- **Color contrast**: Text colors (#F8F8FF, #7B7B8F) on dark BG meet WCAG AA
- **Text size**: FONT_BODY (15px) readable on 320x240
- **Status indicators**: Both color AND shape (dots) distinguish GPS/Radio
- **Alternative**: OLED fallback (monochrome) still fully functional

---

## Summary

The premium UI redesign creates an **elegant, cohesive visual experience** for the ESP32 audit tool:
- 🎨 Sophisticated dark theme with teal accents
- 🎯 Clear visual hierarchy via spacing and elevation
- ⏱️ Smooth, premium animations
- 📐 Generous, well-balanced layout
- 🎪 Consistent design tokens everywhere

**All without changing a single line of tool logic or hardware wiring.**

Ready for Phase 2: Advanced widgets (waterfall, histogram, heatmap) and Phase 3: Enhanced screens with live graphics! 🚀
