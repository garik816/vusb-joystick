#pragma once
#include <stdint.h>

#define GAMEPAD_REPORT_SIZE 7
#define JOY_UP    (1u << 10)
#define JOY_DOWN  (1u << 11)
#define JOY_LEFT  (1u << 12)
#define JOY_RIGHT (1u << 13)
#define JOY_HAT_NEUTRAL 15u

typedef struct {
    uint16_t min;
    uint16_t center;
    uint16_t max;
    uint8_t reverse;
    uint8_t deadzone;
} AxisCalibration;

static inline uint8_t scaleAxis(uint16_t raw, const AxisCalibration *cal) {
    if (cal->max <= cal->min) return 128;
    if (raw < cal->min) raw = cal->min;
    if (raw > cal->max) raw = cal->max;
    const uint16_t span = cal->max - cal->min;
    uint8_t result = (uint8_t)(((uint32_t)(raw - cal->min) * 255u + span / 2u) / span);
    if (cal->deadzone && cal->center >= cal->min && cal->center <= cal->max) {
        const uint16_t lo = cal->center > cal->deadzone ? cal->center - cal->deadzone : 0;
        const uint16_t hi = cal->center + cal->deadzone;
        if (raw >= lo && raw <= hi) result = 128;
    }
    return cal->reverse ? (uint8_t)(255u - result) : result;
}

static inline uint8_t dpadHat(uint16_t buttons) {
    uint8_t up = (buttons & JOY_UP) != 0;
    uint8_t down = (buttons & JOY_DOWN) != 0;
    uint8_t left = (buttons & JOY_LEFT) != 0;
    uint8_t right = (buttons & JOY_RIGHT) != 0;
    if (up && down) up = down = 0;
    if (left && right) left = right = 0;
    if (up) return right ? 1 : left ? 7 : 0;
    if (down) return right ? 3 : left ? 5 : 4;
    if (right) return 2;
    if (left) return 6;
    return JOY_HAT_NEUTRAL;
}

static inline void buildGamepadReport(uint8_t out[GAMEPAD_REPORT_SIZE],
                                      const uint8_t axes[4], uint16_t buttons) {
    out[0] = axes[0];
    out[1] = axes[1];
    out[2] = axes[2];
    out[3] = axes[3];
    out[4] = dpadHat(buttons);
    out[5] = (uint8_t)buttons;
    out[6] = (uint8_t)((buttons >> 8) & 0x03u);
}
