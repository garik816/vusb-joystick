// Freedom Cordless Joystick 2.4 -> USB HID Game Pad (not DualShock 3).
// V-USB: D+ = Arduino D2/PD2, D- = Arduino D3/PD3.
#include <Arduino.h>
#include <EasyHID.h>
#include "joystick_config.h"

static const uint8_t rowPins[4] = {4, 1, 12, 13};
static const uint8_t columnPins[3] = {5, 6, 7};
static const uint8_t extraButtonPins[4] = {11, 10, 9, 8};
static const uint8_t analogPins[4] = {A2, A3, A1, A0};
static const char matrix[4][3] = {
    {'2', 'r', '-'}, {'4', 'l', '5'}, {'6', 'd', '1'}, {'3', 'u', '-'}
};
static uint16_t filtered[4] = {512, 512, 512, 512};
static uint8_t analogValid = 0;
static uint16_t previousRaw = 0;
static uint16_t stableButtons = 0;
static uint8_t analogPhase = 0;
static uint32_t lastAxis = 0, lastButtons = 0, lastReport = 0, lastBlink = 0;
static uint8_t ledState = 0;

static void initInputs() {
    for (uint8_t i = 0; i < 4; ++i) {
        digitalWrite(rowPins[i], HIGH);
        pinMode(rowPins[i], OUTPUT);
        pinMode(extraButtonPins[i], INPUT_PULLUP);
        pinMode(analogPins[i], INPUT);
    }
    for (uint8_t i = 0; i < 3; ++i) pinMode(columnPins[i], INPUT_PULLUP);
    pinMode(A5, OUTPUT);
    digitalWrite(A5, LOW);
}

static void sampleAxis(uint8_t i) {
    uint16_t raw = (uint16_t)analogRead(analogPins[i]);
    if (!(analogValid & (1u << i))) {
        filtered[i] = raw;
        analogValid |= (1u << i);
    } else {
        filtered[i] = (uint16_t)((filtered[i] * 3u + raw + 2u) / 4u);
    }
}

static void updateAxes(uint32_t now) {
    if ((uint32_t)(now - lastAxis) < AXIS_STEP_MS) return;
    lastAxis = now;
    if (analogPhase < 3) {
        sampleAxis(analogPhase++);
    } else if (analogPhase == 3) {
        // PB4/PB5 are shared with the grip matrix. Pause matrix scanning.
        digitalWrite(rowPins[2], LOW);
        digitalWrite(rowPins[3], HIGH);
        analogPhase = 4; // wait at least 2ms for the twist potentiometer
    } else {
        sampleAxis(3);
        digitalWrite(rowPins[2], HIGH);
        analogPhase = 0;
    }
}

static uint16_t scanButtons() {
    uint16_t mask = 0;
    for (uint8_t r = 0; r < 4; ++r) {
        digitalWrite(rowPins[r], LOW);
        delayMicroseconds(3);
        for (uint8_t c = 0; c < 3; ++c) {
            if (digitalRead(columnPins[c]) == LOW) {
                char key = matrix[r][c];
                if (key >= '1' && key <= '6') mask |= (uint16_t)(1u << (key - '1'));
                else if (key == 'u') mask |= JOY_UP;
                else if (key == 'd') mask |= JOY_DOWN;
                else if (key == 'l') mask |= JOY_LEFT;
                else if (key == 'r') mask |= JOY_RIGHT;
            }
        }
        digitalWrite(rowPins[r], HIGH);
    }
    for (uint8_t i = 0; i < 4; ++i)
        if (digitalRead(extraButtonPins[i]) == LOW) mask |= (uint16_t)(1u << (6u + i));
    return mask;
}

static void updateButtons(uint32_t now) {
    if (analogPhase == 4 || (uint32_t)(now - lastButtons) < BUTTON_SCAN_MS) return;
    lastButtons = now;
    const uint16_t current = scanButtons();
    // Per-bit debounce: accept a state after two consecutive equal samples.
    const uint16_t unchanged = (uint16_t)~(current ^ previousRaw);
    stableButtons = (stableButtons & (uint16_t)~unchanged) | (current & unchanged);
    previousRaw = current;
}

static void updateReport() {
    uint8_t axes[4];
    for (uint8_t i = 0; i < 4; ++i)
        axes[i] = scaleAxis(filtered[i], &kAxisCalibration[i]);
    buildGamepadReport(report_buffer, axes, stableButtons);
}

void setup() {
    analogReference(DEFAULT);
    HID.begin(); // configures PD2/PD3, reconnects V-USB
    initInputs();
    report_buffer[0] = report_buffer[1] = report_buffer[2] = report_buffer[3] = 128;
    report_buffer[4] = JOY_HAT_NEUTRAL;
    report_buffer[5] = report_buffer[6] = 0;
}

void loop() {
    HID.tick(); // required continuously for software USB
    const uint32_t now = millis();
    updateAxes(now);
    updateButtons(now);

    if ((uint32_t)(now - lastReport) >= USB_REPORT_MS &&
        analogValid == 0x0f && usbInterruptIsReady()) {
        lastReport = now;
        updateReport();
        usbSetInterrupt(report_buffer, GAMEPAD_REPORT_SIZE);
    }
    if ((uint32_t)(now - lastBlink) >= 1000u) {
        lastBlink = now;
        ledState = !ledState;
        digitalWrite(A5, ledState);
    }
}
