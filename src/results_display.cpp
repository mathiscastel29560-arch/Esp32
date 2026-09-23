#include "results_display.h"
#include "display.h"
#include "ui/theme.h"
#include <TFT_eSPI.h>

namespace ResultsDisplay {

// TFT Display dimensions
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define TITLE_HEIGHT 25
#define STATUS_HEIGHT 20
#define PROGRESS_HEIGHT 10
#define LINE_HEIGHT 18
#define MARGIN 8

// Get color based on result type
uint16_t getResultColor(ResultType type) {
    switch (type) {
        case ResultType::SUCCESS:    return TFT_GREEN;     // ✓ Success
        case ResultType::WARNING:    return TFT_YELLOW;    // ⚠ Warning
        case ResultType::ERROR:      return TFT_RED;       // ✗ Error
        case ResultType::INFO:       return TFT_CYAN;      // ℹ Info
        case ResultType::SCAN_RESULT: return TFT_BLUE;    // 📊 Scan
    }
    return TFT_WHITE;
}

// Get status emoji
String getStatusEmoji(ResultType type) {
    switch (type) {
        case ResultType::SUCCESS:    return "✓ ";
        case ResultType::WARNING:    return "⚠ ";
        case ResultType::ERROR:      return "✗ ";
        case ResultType::INFO:       return "ℹ ";
        case ResultType::SCAN_RESULT: return "📊";
    }
    return "";
}

void showResult(const String& toolName, const DisplayResult& result) {
    if (Display::kind() != Display::ScreenKind::TFT) return;

    TFT_eSPI& tft = Display::raw();
    uint16_t color = getResultColor(result.type);
    String emoji = getStatusEmoji(result.type);

    // Clear screen
    tft.fillScreen(TFT_BLACK);

    // Title bar
    tft.fillRect(0, 0, SCREEN_WIDTH, TITLE_HEIGHT, color);
    tft.setTextColor(TFT_BLACK, color);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);  // Larger title text
    tft.drawString(emoji + result.title, SCREEN_WIDTH / 2, TITLE_HEIGHT / 2);

    // Status line
    tft.setTextColor(color, TFT_BLACK);
    tft.setTextSize(1);  // Default size
    tft.setTextDatum(TL_DATUM);
    tft.drawString(result.status, MARGIN, TITLE_HEIGHT + MARGIN);

    // Progress bar if applicable
    if (result.progress > 0 && result.progress < 100) {
        int16_t barY = TITLE_HEIGHT + STATUS_HEIGHT + 5;
        int16_t barWidth = SCREEN_WIDTH - 2 * MARGIN;
        int16_t barHeight = PROGRESS_HEIGHT;

        // Border
        tft.drawRect(MARGIN, barY, barWidth, barHeight, color);

        // Progress fill
        int16_t fillWidth = (barWidth - 4) * result.progress / 100;
        tft.fillRect(MARGIN + 2, barY + 2, fillWidth, barHeight - 4, color);

        // Percentage text
        String percentStr = String(result.progress) + "%";
        tft.setTextColor(TFT_WHITE);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(percentStr, SCREEN_WIDTH / 2, barY + barHeight / 2);
    }

    // Results content
    int16_t yOffset = TITLE_HEIGHT + STATUS_HEIGHT + PROGRESS_HEIGHT + 15;
    tft.setTextColor(TFT_WHITE);
    tft.setTextDatum(TL_DATUM);

    for (const auto& line : result.lines) {
        if (yOffset + LINE_HEIGHT > SCREEN_HEIGHT) break;  // Stop if out of bounds

        // Truncate long lines
        String displayLine = line;
        if (displayLine.length() > 35) {
            displayLine = displayLine.substring(0, 32) + "...";
        }

        tft.drawString(displayLine, MARGIN, yOffset);
        yOffset += LINE_HEIGHT;
    }

    // Bottom info bar
    tft.fillRect(0, SCREEN_HEIGHT - 20, SCREEN_WIDTH, 20, TFT_DARKGREY);
    tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
    tft.setTextDatum(BC_DATUM);
    tft.drawString(toolName + " | Press BACK to exit", SCREEN_WIDTH / 2, SCREEN_HEIGHT - 5);
}

void updateProgress(int percent, const String& status) {
    if (Display::kind() != Display::ScreenKind::TFT) return;

    TFT_eSPI& tft = Display::raw();

    // Update status
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.fillRect(MARGIN, TITLE_HEIGHT + MARGIN, SCREEN_WIDTH - 2*MARGIN, STATUS_HEIGHT, TFT_BLACK);
    tft.drawString(status, MARGIN, TITLE_HEIGHT + MARGIN);

    // Update progress bar
    int16_t barY = TITLE_HEIGHT + STATUS_HEIGHT + 5;
    int16_t barWidth = SCREEN_WIDTH - 2 * MARGIN;

    // Clear old bar
    tft.fillRect(MARGIN, barY, barWidth, PROGRESS_HEIGHT, TFT_BLACK);
    tft.drawRect(MARGIN, barY, barWidth, PROGRESS_HEIGHT, TFT_CYAN);

    // Draw new progress
    int16_t fillWidth = (barWidth - 4) * percent / 100;
    tft.fillRect(MARGIN + 2, barY + 2, fillWidth, PROGRESS_HEIGHT - 4, TFT_CYAN);

    // Percentage
    String percentStr = String(percent) + "%";
    tft.setTextColor(TFT_WHITE);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(percentStr, SCREEN_WIDTH / 2, barY + PROGRESS_HEIGHT / 2);
}

void clearResults() {
    if (Display::kind() != Display::ScreenKind::TFT) return;

    TFT_eSPI& tft = Display::raw();
    tft.fillScreen(TFT_BLACK);
}

void showTable(const String& title, const std::vector<String>& headers, const std::vector<std::vector<String>>& rows) {
    if (Display::kind() != Display::ScreenKind::TFT) return;

    TFT_eSPI& tft = Display::raw();

    // Header
    tft.fillScreen(TFT_BLACK);
    tft.fillRect(0, 0, SCREEN_WIDTH, TITLE_HEIGHT, TFT_BLUE);
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);
    tft.drawString("📊 " + title, SCREEN_WIDTH / 2, TITLE_HEIGHT / 2);

    // Column headers
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextSize(1);

    int16_t colWidth = SCREEN_WIDTH / headers.size();
    int16_t yPos = TITLE_HEIGHT + 5;

    for (size_t i = 0; i < headers.size(); i++) {
        int16_t xPos = i * colWidth + 5;
        tft.drawString(headers[i], xPos, yPos);
    }

    yPos += LINE_HEIGHT;

    // Data rows
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    for (const auto& row : rows) {
        if (yPos + LINE_HEIGHT > SCREEN_HEIGHT - 20) break;

        for (size_t i = 0; i < row.size() && i < headers.size(); i++) {
            int16_t xPos = i * colWidth + 5;
            String cell = row[i];
            if (cell.length() > colWidth / 6) {
                cell = cell.substring(0, colWidth / 6 - 2) + "..";
            }
            tft.drawString(cell, xPos, yPos);
        }
        yPos += LINE_HEIGHT;
    }

    // Footer
    tft.fillRect(0, SCREEN_HEIGHT - 20, SCREEN_WIDTH, 20, TFT_DARKGREY);
    tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
    tft.setTextDatum(BC_DATUM);
    tft.drawString("Results: " + String(rows.size()) + " items", SCREEN_WIDTH / 2, SCREEN_HEIGHT - 5);
}

void showSignalBar(const String& label, int rssi) {
    if (Display::kind() != Display::ScreenKind::TFT) return;

    TFT_eSPI& tft = Display::raw();

    // Label
    tft.setTextColor(TFT_WHITE);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(label + ":", MARGIN, MARGIN);

    // Signal bars (0-5)
    int16_t barCount = 0;
    uint16_t barColor = TFT_RED;

    if (rssi > -50) {
        barCount = 5;
        barColor = TFT_GREEN;
    } else if (rssi > -60) {
        barCount = 4;
        barColor = TFT_GREEN;
    } else if (rssi > -70) {
        barCount = 3;
        barColor = TFT_YELLOW;
    } else if (rssi > -80) {
        barCount = 2;
        barColor = TFT_ORANGE;
    } else {
        barCount = 1;
        barColor = TFT_RED;
    }

    int16_t barX = SCREEN_WIDTH - 60;
    for (int i = 0; i < 5; i++) {
        uint16_t color = (i < barCount) ? barColor : TFT_DARKGREY;
        int16_t barHeight = 4 + (i * 3);
        tft.fillRect(barX + i * 10, MARGIN + 15 - barHeight, 8, barHeight, color);
    }

    // RSSI value
    tft.setTextColor(TFT_CYAN);
    tft.drawString(String(rssi) + "dBm", barX - 5, MARGIN + 20);
}

void showFrequencyInfo(uint16_t frequency, int8_t rssi, uint8_t channel) {
    if (Display::kind() != Display::ScreenKind::TFT) return;

    TFT_eSPI& tft = Display::raw();

    // Frequency
    tft.setTextColor(TFT_CYAN);
    tft.drawString("Freq: " + String(frequency) + "MHz", MARGIN, 50);

    // Channel
    tft.setTextColor(TFT_YELLOW);
    tft.drawString("Ch: " + String(channel), MARGIN, 70);

    // RSSI with bar
    showSignalBar("RSSI", rssi);
}

}  // namespace ResultsDisplay
