#include "results_formatter.h"
#include "debug_logger.h"

namespace ResultsFormatter {

void displayResult(const String &title, const String &description,
                   ResultType type, uint8_t successPercent) {
    String icon = "ℹ️ ";
    String color = COLOR_CYAN;
    String topBorder = "╔";
    String middleBorder = "╠";
    String bottomBorder = "╚";

    switch (type) {
        case RESULT_SUCCESS:
            icon = "✓ ";
            color = COLOR_GREEN;
            break;
        case RESULT_WARNING:
            icon = "⚠ ";
            color = COLOR_YELLOW;
            break;
        case RESULT_ERROR:
            icon = "✗ ";
            color = COLOR_RED;
            break;
        case RESULT_INFO:
            icon = "ℹ ";
            color = COLOR_CYAN;
            break;
        case RESULT_SCAN:
            icon = "📊 ";
            color = COLOR_BLUE;
            break;
    }

    Serial.println();
    Serial.print(color);
    Serial.println(topBorder + String(41, '═') + "╗");
    Serial.print("║ " + icon);
    Serial.print(COLOR_GREEN);
    Serial.print(title);
    Serial.print(color);
    Serial.println(String(36 - title.length(), ' ') + "║");
    Serial.println(middleBorder + String(41, '═') + "╣");
    Serial.print(COLOR_RESET);

    // Print description with wrapping
    int lines = 0;
    int pos = 0;
    while (pos < description.length() && lines < 6) {
        int nextNewline = description.indexOf('\n', pos);
        if (nextNewline == -1) nextNewline = description.length();

        String line = description.substring(pos, nextNewline);
        if (line.length() > 39) line = line.substring(0, 39);

        Serial.print("║ ");
        Serial.print(line);
        Serial.println(String(39 - line.length(), ' ') + "║");

        pos = nextNewline + 1;
        lines++;
    }

    // Show success bar if applicable
    if (type == RESULT_SCAN || type == RESULT_SUCCESS) {
        Serial.println(middleBorder + String(41, '═') + "╣");
        Serial.print("║ ");
        uint8_t filled = successPercent / 10;
        for (uint8_t i = 0; i < 10; i++) {
            if (i < filled) Serial.print(COLOR_GREEN + String("█") + COLOR_RESET);
            else Serial.print("░");
        }
        Serial.print(" " + String(successPercent) + "%");
        Serial.println(String(28 - String(successPercent).length(), ' ') + "║");
    }

    Serial.print(color);
    Serial.println(bottomBorder + String(41, '═') + "╝");
    Serial.print(COLOR_RESET);
}

void displayStats(const String &title, const std::vector<StatEntry> &stats) {
    Serial.println();
    Serial.print(COLOR_BLUE);
    Serial.println("╔═════════════════════════════════════════╗");
    Serial.print("║ 📊 ");
    Serial.print(COLOR_GREEN);
    Serial.print(title);
    Serial.print(COLOR_BLUE);
    Serial.println(String(35 - title.length(), ' ') + "║");
    Serial.println("╠═════════════════════════════════════════╣");
    Serial.print(COLOR_RESET);

    for (size_t i = 0; i < stats.size(); i++) {
        String line = stats[i].label + ": ";
        Serial.print("║ ");
        Serial.print(COLOR_YELLOW);
        Serial.print(stats[i].value);
        Serial.print(COLOR_RESET);
        if (stats[i].unit.length() > 0) {
            Serial.print(stats[i].unit);
        }
        Serial.print(COLOR_CYAN);
        Serial.println(String(37 - line.length() - stats[i].value.length(), ' ') + "║");
    }

    Serial.print(COLOR_BLUE);
    Serial.println("╚═════════════════════════════════════════╝");
    Serial.print(COLOR_RESET);
}

void displayScanResults(const String &title, const std::vector<ScanEntry> &results) {
    Serial.println();
    Serial.print(COLOR_BLUE);
    Serial.println("╔═════════════════════════════════════════╗");
    Serial.print("║ 🔍 ");
    Serial.print(COLOR_GREEN);
    Serial.print(title);
    Serial.print(COLOR_BLUE);
    Serial.println(String(35 - title.length(), ' ') + "║");
    Serial.println("╠═════════════════════════════════════════╣");

    for (size_t i = 0; i < results.size() && i < 8; i++) {
        String name = results[i].name;
        if (name.length() > 20) name = name.substring(0, 17) + "...";

        Serial.print("║ ");
        Serial.print(COLOR_CYAN);
        Serial.print(name);
        Serial.print(COLOR_RESET);

        // Signal indicator
        int signalVal = results[i].signal.toInt();
        if (signalVal > 75) Serial.print(COLOR_GREEN + "███░░" + COLOR_RESET);
        else if (signalVal > 50) Serial.print(COLOR_YELLOW + "██░░░" + COLOR_RESET);
        else Serial.print(COLOR_RED + "█░░░░" + COLOR_RESET);

        Serial.print(" ");
        Serial.print(results[i].signal);
        Serial.println(String(28 - name.length() - results[i].signal.length(), ' ') + "║");
    }

    if (results.size() > 8) {
        Serial.print(COLOR_YELLOW);
        Serial.println("║ ... and " + String(results.size() - 8) + " more results");
        Serial.print(COLOR_RESET);
    }

    Serial.print(COLOR_BLUE);
    Serial.println("╚═════════════════════════════════════════╝");
    Serial.print(COLOR_RESET);
}

void displayTable(const String &title, const std::vector<String> &headers,
                  const std::vector<std::vector<String>> &rows) {
    Serial.println();
    Serial.print(COLOR_BLUE);
    Serial.println("╔═════════════════════════════════════════╗");
    Serial.print("║ 📋 ");
    Serial.print(COLOR_GREEN);
    Serial.print(title);
    Serial.print(COLOR_BLUE);
    Serial.println(String(35 - title.length(), ' ') + "║");
    Serial.println("╠═════════════════════════════════════════╣");

    // Print headers
    Serial.print(COLOR_YELLOW);
    Serial.print("║ ");
    for (size_t i = 0; i < headers.size() && i < 3; i++) {
        Serial.print(headers[i].substring(0, 12));
        Serial.print(" │ ");
    }
    Serial.println();
    Serial.println(COLOR_BLUE + String("╠") + String(41, '═') + "╣" + COLOR_RESET);

    // Print rows
    for (size_t i = 0; i < rows.size() && i < 6; i++) {
        Serial.print(COLOR_CYAN);
        Serial.print("║ ");
        for (size_t j = 0; j < rows[i].size() && j < 3; j++) {
            String cell = rows[i][j];
            if (cell.length() > 10) cell = cell.substring(0, 7) + "...";
            Serial.print(cell);
            Serial.print(String(12 - cell.length(), ' '));
            Serial.print(" │ ");
        }
        Serial.println();
    }

    Serial.print(COLOR_BLUE);
    Serial.println("╚═════════════════════════════════════════╝");
    Serial.print(COLOR_RESET);
}

void startProgress(const String &task, uint8_t steps) {
    Serial.println();
    Serial.print(COLOR_GREEN);
    Serial.println("╔═════════════════════════════════════════╗");
    Serial.print("║ ⏳ ");
    Serial.print(task);
    Serial.println(String(37 - task.length(), ' ') + "║");
    Serial.print(COLOR_RESET);
}

void updateProgress(uint8_t step, const String &description) {
    if (description.length() > 0) {
        Serial.print(COLOR_CYAN);
        Serial.print("║ Step " + String(step) + ": ");
        Serial.print(description);
        Serial.println(String(28 - description.length(), ' ') + "║");
    }

    Serial.print("║ [");
    for (uint8_t i = 0; i < 10; i++) {
        if (i < step) Serial.print(COLOR_GREEN + "█" + COLOR_RESET);
        else Serial.print("░");
    }
    Serial.println("] " + String(step * 10) + "%                  ║");
}

void endProgress() {
    Serial.print(COLOR_GREEN);
    Serial.println("╚═════════════════════════════════════════╝");
    Serial.print(COLOR_RESET);
}

void displaySummary(const String &title, uint32_t total, uint32_t success,
                    uint32_t failed, uint32_t skipped) {
    Serial.println();
    Serial.print(COLOR_BLUE);
    Serial.println("╔═════════════════════════════════════════╗");
    Serial.print("║ 📊 ");
    Serial.print(COLOR_GREEN);
    Serial.print(title + " Summary");
    Serial.print(COLOR_BLUE);
    Serial.println(String(32 - title.length(), ' ') + "║");
    Serial.println("╠═════════════════════════════════════════╣");

    Serial.print("║ Total:   ");
    Serial.print(COLOR_CYAN);
    Serial.print(String(total));
    Serial.print(COLOR_RESET);
    Serial.println(String(28 - String(total).length(), ' ') + "║");

    Serial.print("║ Success: ");
    Serial.print(COLOR_GREEN);
    Serial.print(String(success));
    Serial.print(COLOR_RESET);
    Serial.println(String(28 - String(success).length(), ' ') + "║");

    Serial.print("║ Failed:  ");
    Serial.print(COLOR_RED);
    Serial.print(String(failed));
    Serial.print(COLOR_RESET);
    Serial.println(String(28 - String(failed).length(), ' ') + "║");

    if (skipped > 0) {
        Serial.print("║ Skipped: ");
        Serial.print(COLOR_YELLOW);
        Serial.print(String(skipped));
        Serial.print(COLOR_RESET);
        Serial.println(String(28 - String(skipped).length(), ' ') + "║");
    }

    uint8_t percent = total > 0 ? (success * 100) / total : 0;
    Serial.println("╠═════════════════════════════════════════╣");
    Serial.print("║ ");
    for (uint8_t i = 0; i < 10; i++) {
        if (i < (percent / 10)) Serial.print(COLOR_GREEN + "█" + COLOR_RESET);
        else Serial.print("░");
    }
    Serial.print(" " + String(percent) + "%");
    Serial.println(String(24 - String(percent).length(), ' ') + "║");

    Serial.print(COLOR_BLUE);
    Serial.println("╚═════════════════════════════════════════╝");
    Serial.print(COLOR_RESET);
}

void displaySignalMap(const String &channel, int rssi) {
    String bars = "";
    String indicator = "";

    if (rssi > -30) {
        bars = "██████████";
        indicator = "Excellent";
    } else if (rssi > -50) {
        bars = "████████░░";
        indicator = "Very Good";
    } else if (rssi > -60) {
        bars = "██████░░░░";
        indicator = "Good";
    } else if (rssi > -70) {
        bars = "████░░░░░░";
        indicator = "Fair";
    } else if (rssi > -80) {
        bars = "██░░░░░░░░";
        indicator = "Weak";
    } else {
        bars = "░░░░░░░░░░";
        indicator = "Very Weak";
    }

    Serial.print(COLOR_CYAN);
    Serial.print("║ " + channel + ": [");
    Serial.print(COLOR_GREEN);
    Serial.print(bars);
    Serial.print(COLOR_CYAN);
    Serial.print("] " + String(rssi) + "dBm (" + indicator + ")");
    Serial.println(String(26 - channel.length() - indicator.length(), ' ') + "║");
    Serial.print(COLOR_RESET);
}

}  // namespace ResultsFormatter
