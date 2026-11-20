#include <cassert>
#include <cstdint>
#include "../joystik/joystick_logic.h"

int main() {
    const AxisCalibration cal = {0, 512, 1023, 0, 0};
    const AxisCalibration rev = {0, 512, 1023, 1, 0};
    const AxisCalibration zone = {0, 512, 1023, 0, 8};
    const AxisCalibration invalid = {100, 100, 100, 0, 0};
    assert(scaleAxis(0, &cal) == 0);
    assert(scaleAxis(1023, &cal) == 255);
    assert(scaleAxis(512, &cal) == 128);
    assert(scaleAxis(0, &rev) == 255 && scaleAxis(1023, &rev) == 0);
    assert(scaleAxis(510, &zone) == 128);
    assert(scaleAxis(501, &zone) != 128);
    assert(scaleAxis(900, &invalid) == 128);
    for (uint16_t i = 1; i <= 1023; ++i) assert(scaleAxis(i, &cal) >= scaleAxis(i - 1, &cal));

    assert(dpadHat(0) == 15);
    assert(dpadHat(JOY_UP) == 0);
    assert(dpadHat(JOY_UP | JOY_RIGHT) == 1);
    assert(dpadHat(JOY_RIGHT) == 2);
    assert(dpadHat(JOY_DOWN | JOY_RIGHT) == 3);
    assert(dpadHat(JOY_DOWN) == 4);
    assert(dpadHat(JOY_DOWN | JOY_LEFT) == 5);
    assert(dpadHat(JOY_LEFT) == 6);
    assert(dpadHat(JOY_UP | JOY_LEFT) == 7);
    assert(dpadHat(JOY_UP | JOY_DOWN) == 15);
    assert(dpadHat(JOY_LEFT | JOY_RIGHT) == 15);

    uint8_t axes[4] = {0, 127, 255, 200};
    uint8_t out[GAMEPAD_REPORT_SIZE] = {};
    buildGamepadReport(out, axes, (1u << 0) | (1u << 9) | JOY_RIGHT);
    assert(sizeof(out) == 7);
    assert(out[0] == 0 && out[1] == 127 && out[2] == 255 && out[3] == 200);
    assert(out[4] == 2 && out[5] == 1 && out[6] == 2);
    buildGamepadReport(out, axes, 0);
    assert(out[4] == 15 && out[5] == 0 && out[6] == 0);
}
