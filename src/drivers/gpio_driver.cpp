#include "drivers/gpio_driver.h"
#include "hw_config.h"

namespace GPIODriver {

static bool initialized = false;
static uint32_t buttonPressTime[4] = {0, 0, 0, 0};
static bool buttonPressed[4] = {false, false, false, false};

bool init() {
    if (initialized) return true;

    Serial.println("[GPIO] Initializing pins...");

    // Buttons
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_OK, INPUT_PULLUP);
    pinMode(BTN_BACK, INPUT_PULLUP);

    // Buzzer (PWM)
    ledcSetup(BUZZER_CHANNEL, BUZZER_FREQ, 8);
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
    ledcWrite(BUZZER_CHANNEL, 0);  // Off

    // IR LED
    pinMode(IR_TX_PIN, OUTPUT);
    digitalWrite(IR_TX_PIN, LOW);

    // IR Receiver
    pinMode(IR_RX_PIN, INPUT);

    // Battery ADC
    analogReadResolution(12);  // 12-bit ADC

    initialized = true;
    Serial.println("[GPIO] ✓ Initialized");
    return true;
}

ButtonEvent getButtonState(uint8_t btnPin) {
    if (!initialized) return BUTTON_NONE;

    bool isPressed = (digitalRead(btnPin) == LOW);  // Active LOW
    uint8_t btnIndex = 0;

    switch (btnPin) {
        case BTN_UP: btnIndex = 0; break;
        case BTN_DOWN: btnIndex = 1; break;
        case BTN_OK: btnIndex = 2; break;
        case BTN_BACK: btnIndex = 3; break;
        default: return BUTTON_NONE;
    }

    // Debounce
    if (isPressed && !buttonPressed[btnIndex]) {
        buttonPressed[btnIndex] = true;
        buttonPressTime[btnIndex] = millis();
        return BUTTON_PRESSED;
    } else if (!isPressed && buttonPressed[btnIndex]) {
        buttonPressed[btnIndex] = false;

        // Check for long press
        if ((millis() - buttonPressTime[btnIndex]) > BUTTON_LONG_PRESS_MS) {
            return BUTTON_LONG_PRESS;
        }

        return BUTTON_RELEASED;
    }

    return BUTTON_NONE;
}

bool isButtonPressed(uint8_t btnPin) {
    if (!initialized) return false;
    return (digitalRead(btnPin) == LOW);
}

void buzzerBeep(uint32_t durationMs) {
    if (!initialized) return;

    ledcWrite(BUZZER_CHANNEL, 128);  // 50% duty cycle
    delay(durationMs);
    ledcWrite(BUZZER_CHANNEL, 0);
}

void buzzerTone(uint16_t frequency, uint32_t durationMs) {
    if (!initialized) return;

    ledcChangeFrequency(BUZZER_CHANNEL, frequency, 8);
    ledcWrite(BUZZER_CHANNEL, 128);
    delay(durationMs);
    ledcWrite(BUZZER_CHANNEL, 0);
    ledcChangeFrequency(BUZZER_CHANNEL, BUZZER_FREQ, 8);
}

void buzzerPattern(const uint16_t* durations, uint8_t count) {
    if (!initialized || !durations) return;

    for (uint8_t i = 0; i < count; i++) {
        buzzerBeep(durations[i]);
        if (i < count - 1) delay(100);
    }
}

void irLedOn(uint16_t frequency) {
    if (!initialized) return;

    ledcSetup(1, frequency, 8);  // PWM channel 1
    ledcAttachPin(IR_TX_PIN, 1);
    ledcWrite(1, 128);  // 50% duty cycle
}

void irLedOff() {
    if (!initialized) return;

    ledcWrite(1, 0);
    ledcDetachPin(IR_TX_PIN);
    digitalWrite(IR_TX_PIN, LOW);
}

uint8_t getBatteryPercent() {
    if (!initialized) return 0;

    // Read ADC and convert to percentage
    uint16_t adcVal = 0;
    for (uint8_t i = 0; i < BATTERY_SAMPLES; i++) {
        adcVal += analogRead(BATTERY_ADC_PIN);
        delay(10);
    }
    adcVal /= BATTERY_SAMPLES;

    // Calibration: 2048 = ~3.3V, 4095 = ~6.6V (with 2x divider)
    // Assuming 3.0V min, 4.2V max for LiPo
    float voltage = (adcVal / 4095.0f) * 6.6f;

    uint8_t percent = 0;
    if (voltage >= 4.2f) {
        percent = 100;
    } else if (voltage <= 3.0f) {
        percent = 0;
    } else {
        percent = (uint8_t)((voltage - 3.0f) / (4.2f - 3.0f) * 100);
    }

    return percent;
}

float getBatteryVoltage() {
    if (!initialized) return 0.0f;

    uint16_t adcVal = 0;
    for (uint8_t i = 0; i < BATTERY_SAMPLES; i++) {
        adcVal += analogRead(BATTERY_ADC_PIN);
        delay(10);
    }
    adcVal /= BATTERY_SAMPLES;

    return (adcVal / 4095.0f) * 6.6f;  // 2x voltage divider
}

bool isBatteryLow() {
    return (getBatteryPercent() < BATTERY_LOW_THRESHOLD);
}

}  // namespace GPIODriver
