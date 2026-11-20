#pragma once
#include "joystick_logic.h"

// X=A2, Y=A3, Rx=A1 (throttle), Ry=A0 (twist).
// Adjust min/center/max after measuring the real potentiometer endpoints.
static const AxisCalibration kAxisCalibration[4] = {
    {0, 512, 1023, 0, 8},
    {0, 512, 1023, 0, 8},
    {0, 512, 1023, 1, 0},
    {0, 512, 1023, 0, 8},
};
#define BUTTON_SCAN_MS 5u
#define AXIS_STEP_MS 2u
#define USB_REPORT_MS 10u
