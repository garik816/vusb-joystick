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

    extern uint8_t report_buffer[8];

    void usbReportSend(uint8_t sz);

#ifdef __cplusplus
}
#endif

#endif