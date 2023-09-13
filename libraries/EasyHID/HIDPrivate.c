#include "HIDPrivate.h"

uint8_t report_buffer[8];
uint8_t idle_rate = 500 / 4;  	// see HID1_11.pdf sect 7.2.4
uint8_t protocol_version = 0; 	// see HID1_11.pdf sect 7.2.6

void usbReportSend(uint8_t sz)
{
    // perform usb background tasks until the report can be sent, then send it
    while (1)
    {
        usbPoll(); // this needs to be called at least once every 10 ms
        if (usbInterruptIsReady())
        {
            usbSetInterrupt((uint8_t*)report_buffer, sz); // send
            break;

            // see http://vusb.wikidot.com/driver-api
        }
    }
}

// USB HID report descriptor for boot protocol keyboard
// see HID1_11.pdf appendix B section 1
// USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH is defined in usbconfig (should be 78)
const PROGMEM char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
        0x05, 0x01,     // USAGE_PAGE (Generic Desktop)
        0x09, 0x05,     // USAGE (Game Pad)
        0xa1, 0x01,     // COLLECTION (Application)

        0xa1, 0x00,     //   COLLECTION (Physical)

        0x05, 0x01,     //	   USAGE_PAGE (Generic Desktop)
        0x09, 0x30,     //     USAGE (X)
        0x09, 0x31,     //     USAGE (Y)
        0x09, 0x33,		//     USAGE (Rx)
        0x09, 0x34,		//     USAGE (Ry)
        0x15, 0x81,     //     LOGICAL_MINIMUM (-127)
        0x25, 0x7F,     //     LOGICAL_MAXIMUM (127)
        0x75, 0x08,     //     REPORT_SIZE (8)
        0x95, 0x04,     //     REPORT_COUNT (4)
        0x81, 0x02,     //     INPUT (Data,Var,Abs)

        0x05, 0x09,     // 	   USAGE_PAGE (Button)
        0x19, 0x01,     //     USAGE_MINIMUM (Button 1)
        0x29, 0x08,     //     USAGE_MAXIMUM (Button 8)
        0x15, 0x00,     //     LOGICAL_MINIMUM (0)
        0x25, 0x01,     //     LOGICAL_MAXIMUM (1)
        0x75, 0x01,     //     REPORT_SIZE (1)
        0x95, 0x08,     //     REPORT_COUNT (8)
        0x81, 0x02,     //     INPUT (Data,Var,Abs)

        0x05, 0x09,     // 	   USAGE_PAGE (Button)
        0x19, 0x01,     //     USAGE_MINIMUM (Button 1)
        0x29, 0x02,     //     USAGE_MAXIMUM (Button 2)
        0x15, 0x00,     //     LOGICAL_MINIMUM (0)
        0x25, 0x01,     //     LOGICAL_MAXIMUM (1)
        0x75, 0x01,     //     REPORT_SIZE (1)
        0x95, 0x08,     //     REPORT_COUNT (8)
        0x81, 0x02,     //     INPUT (Data,Var,Abs)

        0x05, 0x09,     // 	   USAGE_PAGE (Button)
        0x19, 0x01,     //     USAGE_MINIMUM (Button 1)
        0x29, 0x04,     //     USAGE_MAXIMUM (Button 4)
        0x15, 0x00,     //     LOGICAL_MINIMUM (0)
        0x25, 0x01,     //     LOGICAL_MAXIMUM (1)
        0x75, 0x01,     //     REPORT_SIZE (1)
        0x95, 0x08,     //     REPORT_COUNT (8)
        0x81, 0x02,     //     INPUT (Data,Var,Abs)

//        0x09, 0x39,     //     USAGE (Hat Switch)
//        0x15, 0x00,     //     LOGICAL_MINIMUM (0)
//        0x25, 0x03,     //     LOGICAL_MAXIMUM (7)
//        0x35, 0x00,     //     PHYSICAL_MINIMUM (0)
//        0x46, 0x0E, 0x01, //   PHYSICAL_MAXIMUM (270)
//        0x65, 0x14,     //     UNIT (Eng Rot:Degree)
//        0x75, 0x04,     //     REPORT_SIZE (4)
//        0x95, 0x01,     //     REPORT_COUNT (1)
//        0x81, 0x02,     //0x42,     //     INPUT (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)


        0xc0,           // 	 END_COLLECTION
        0xc0,           // END_COLLECTION
};


usbMsgLen_t usbFunctionSetup(uchar data[8])
{
    usbRequest_t    *rq = (void *)data;

    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */

        if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
            /* we only have one reportCurr type, so don't look at wValue */
            usbMsgPtr = (void *)&report_buffer;
            return sizeof(report_buffer);
        }else if(rq->bRequest == USBRQ_HID_GET_IDLE){
            usbMsgPtr = &idle_rate;
            return 1;
        }else if(rq->bRequest == USBRQ_HID_SET_IDLE){
            idle_rate = rq->wValue.bytes[1];
        }
    }else{
        /* no vendor specific requests implemented */
    }
    return 0;   /* default for not implemented requests: return no data back to host */
}

// see http://vusb.wikidot.com/driver-api
usbMsgLen_t usbFunctionWrite(uint8_t * data, uchar len)
{
//	if (data[0] == REPID_KEYBOARD)
//		led_state = data[1];
    return 2; // 1 byte read
}

