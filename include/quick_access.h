#ifndef QUICK_ACCESS_H
#define QUICK_ACCESS_H

#include <Arduino.h>
#include <vector>

// Quick access shortcuts for frequently used tools
class QuickAccess {
public:
    struct Shortcut {
        String name;
        String toolName;
        uint32_t callCount;
        bool enabled;
    };

    static QuickAccess& instance() {
        static QuickAccess qa;
        return qa;
    }

    // Add shortcut
    void addShortcut(const char* name, const char* toolName) {
        if (shortcuts.size() >= MAX_SHORTCUTS) {
            shortcuts.erase(shortcuts.begin());  // Remove oldest
        }

        Shortcut sc = {String(name), String(toolName), 0, true};
        shortcuts.push_back(sc);
    }

    // Get shortcuts
    const std::vector<Shortcut>& getShortcuts() const {
        return shortcuts;
    }

    // Execute shortcut
    void executeShortcut(size_t idx) {
        if (idx < shortcuts.size()) {
            shortcuts[idx].callCount++;
            Serial.printf("Executing: %s (%s)\n",
                         shortcuts[idx].name.c_str(),
                         shortcuts[idx].toolName.c_str());
            // Would call actual tool here
        }
    }

    // Toggle shortcut
    void toggleShortcut(size_t idx) {
        if (idx < shortcuts.size()) {
            shortcuts[idx].enabled = !shortcuts[idx].enabled;
        }
    }

    // Display shortcuts
    void displayShortcuts() {
        if (shortcuts.empty()) {
            Serial.println("No shortcuts configured");
            return;
        }

        Serial.println("\n╔════════════════════════════════════════╗");
        Serial.println("║          QUICK ACCESS SHORTCUTS        ║");
        Serial.println("╠════════════════════════════════════════╣");

        for (size_t i = 0; i < shortcuts.size(); i++) {
            const auto& sc = shortcuts[i];
            Serial.printf("║ [%zu] ", i + 1);

            // Enabled/disabled indicator
            if (sc.enabled) {
                Serial.print("✓ ");
            } else {
                Serial.print("  ");
            }

            Serial.print(sc.name);
            Serial.printf(" → %s\n", sc.toolName.c_str());
            Serial.printf("║     (used %lu times)\n", sc.callCount);
        }

        Serial.println("╚════════════════════════════════════════╝\n");
    }

    // Remove shortcut
    void removeShortcut(size_t idx) {
        if (idx < shortcuts.size()) {
            shortcuts.erase(shortcuts.begin() + idx);
        }
    }

    // Clear all
    void clearAll() {
        shortcuts.clear();
    }

private:
    static constexpr uint32_t MAX_SHORTCUTS = 10;
    std::vector<Shortcut> shortcuts;

    QuickAccess() {
        // Add some default shortcuts
        addShortcut("WiFi Scan", "WiFiTools::scan");
        addShortcut("BLE Scan", "BLETools::scan");
        addShortcut("RF Scan", "RF24Tools::scan");
    }
};

#endif
