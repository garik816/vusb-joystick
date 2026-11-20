#ifndef _HIDPrivate_h
#define _HIDPrivate_h
#ifdef __cplusplus
extern "C" {
#endif
#include "usbconfig.h"
#include "usbdrv/usbdrv.h"
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <util/delay.h>
#include <stdint.h>
#define GAMEPAD_REPORT_SIZE 7
extern uint8_t report_buffer[GAMEPAD_REPORT_SIZE];
#ifdef __cplusplus
}
#endif
#endif
