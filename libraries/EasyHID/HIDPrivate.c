#include "HIDPrivate.h"

// Unnumbered 7-byte HID input report; layout must match buildGamepadReport().
uint8_t report_buffer[GAMEPAD_REPORT_SIZE] = {128, 128, 128, 128, 15, 0, 0};
static uint8_t idle_rate = 0;

const PROGMEM char usbHidReportDescriptor[] = {
    0x05, 0x01, 0x09, 0x05, 0xa1, 0x01,
    0x09, 0x30, 0x09, 0x31, 0x09, 0x33, 0x09, 0x34, 0x15, 0x00, 0x26, 0xff, 0x00, 0x75, 0x08, 0x95, 0x04, 0x81, 0x02,
    0x09, 0x39, 0x15, 0x00, 0x25, 0x07, 0x35, 0x00, 0x46, 0x3b, 0x01, 0x65, 0x14, 0x75, 0x04, 0x95, 0x01, 0x81, 0x42,
    0x65, 0x00, 0x75, 0x04, 0x95, 0x01, 0x81, 0x03,
    0x05, 0x09, 0x19, 0x01, 0x29, 0x0a, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x0a, 0x81, 0x02,
    0x75, 0x06, 0x95, 0x01, 0x81, 0x03,
    0xc0,
};
typedef char hid_descriptor_length_check[
    (sizeof(usbHidReportDescriptor) == USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH) ? 1 : -1
];

usbMsgLen_t usbFunctionSetup(uchar data[8]) {
    usbRequest_t *rq = (usbRequest_t *)data;
    if ((rq->bmRequestType & USBRQ_TYPE_MASK) != USBRQ_TYPE_CLASS) return 0;
    switch (rq->bRequest) {
        case USBRQ_HID_GET_REPORT:
            if (rq->wValue.bytes[1] != 1 || rq->wValue.bytes[0] != 0) return 0;
            usbMsgPtr = (uchar *)report_buffer;
            return GAMEPAD_REPORT_SIZE;
        case USBRQ_HID_GET_IDLE:
            usbMsgPtr = &idle_rate;
            return 1;
        case USBRQ_HID_SET_IDLE:
            idle_rate = rq->wValue.bytes[1];
            return 0;
        default:
            return 0;
    }
}
