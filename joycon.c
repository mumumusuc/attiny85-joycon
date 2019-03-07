#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include "wiring.h"
#include "usbdrv.h"
#include "joycon.h"

/*
const unsigned char gcn64_usbHidReportDescriptor[] PROGMEM = {
        0x05, 0x01,                                         // USAGE_PAGE (Generic Desktop)
        0x09, 0x05,                                         // USAGE (Gamepad)
        0xa1, 0x01,                                         // COLLECTION (Application)
        0x09, 0x01,                                         //		USAGE (Pointer)
        0xa1, 0x00,                                         //		COLLECTION (Physical)
        0x05, 0x01,                                         //			USAGE_PAGE (Generic Desktop)
        0x09, 0x30,                                         //			USAGE (X)
        0x09, 0x31,                                         //			USAGE (Y)
        0x09, 0x33,                         //			USAGE (Rx)
        0x09, 0x34,                        //		USAGE (Ry)
        0x09, 0x35,                        //		USAGE (Rz)
        0x09, 0x36,                        //		USAGE (Slider)
        0x15, 0x00,                                         //			LOGICAL_MINIMUM (0)
        0x26, 0xFF, 0x00,                             //			LOGICAL_MAXIMUM (255)
        0x75, 0x08,                                         //			REPORT_SIZE (8)
        0x95, 0x06,                                         //			REPORT_COUNT (6)
        0x81, 0x02,                                         //			INPUT (Data,Var,Abs)
        0xc0,                                                     //		END_COLLECTION (Physical)
        0x05, 0x09,                                         //		USAGE_PAGE (Button)
        0x19, 0x01,                                         //		USAGE_MINIMUM (Button 1)
        0x29, 0x10,                                         //		USAGE_MAXIMUM (Button 14)
        0x15, 0x00,                                         //		LOGICAL_MINIMUM (0)
        0x25, 0x01,                                         //		LOGICAL_MAXIMUM (1)
        0x75, 0x01,                                         //		REPORT_SIZE (1)
        0x95, 0x10,                                         //		REPORT_COUNT (16)
        0x81, 0x02,                                         //		INPUT (Data,Var,Abs)
        0xc0                                                     // END_COLLECTION (Application)
};
*/
const unsigned char gcn64_usbHidReportDescriptor[] PROGMEM = {
        0x05, 0x01,
        0x09, 0x05,
        0xA1, 0x01,
        // Buttons (2 bytes)
        0x15, 0x00,
        0x25, 0x01,
        0x35, 0x00,
        0x45, 0x01,
        // The Switch will allow us to expand the original HORI descriptors to a full 16 buttons.
        // The Switch will make use of 14 of those buttons.
        0x75, 0x01,
        0x95, 0x10,
        0x05, 0x09,
        0x19, 0x01,
        0x29, 0x10,
        0x81, 0x02,
        // HAT Switch (1 nibble)
        0x05, 0x01,
        0x25, 0x07,
        0x46, 0x3B, 0x01,
        0x75, 0x04,
        0x95, 0x01,
        0x65, 0x14,
        0x09, 0x39,
        0x81, 0x42,
        // There's an additional nibble here that's utilized as part of the Switch Pro Controller.
        // I believe this -might- be separate U/D/L/R bits on the Switch Pro Controller, as they're utilized as four button descriptors on the Switch Pro Controller.
        0x65, 0x00,
        0x95, 0x01,
        0x81, 0x01,
        // Joystick (4 bytes)
        0x26, 0xFF, 0x00,
        0x46, 0xFF, 0x00,
        0x09, 0x30,
        0x09, 0x31,
        0x09, 0x32,
        0x09, 0x35,
        0x75, 0x08,
        0x95, 0x04,
        0x81, 0x02,
        // Vendor Specific (1 byte)
        // This byte requires additional investigation.
        0x06, 0x00, 0xFF,
        0x09, 0x20,
        0x95, 0x01,
        0x81, 0x02,
        // Output (8 bytes)
        // Original observation of this suggests it to be a mirror of the inputs that we sent.
        // The Switch requires us to have these descriptors available.
        0x0A, 0x21, 0x26,
        0x95, 0x08,
        0x91, 0x02,
        0xC0,
};

const unsigned char usbDescrDevice[] PROGMEM = {        /* USB device descriptor */
        18,                     /* sizeof(usbDescrDevice): length of descriptor in bytes */
        USBDESCR_DEVICE,        /* descriptor type */
        0x00, 0x02,             /* USB version supported */
        USB_CFG_DEVICE_CLASS,
        USB_CFG_DEVICE_SUBCLASS,
        0,                      /* protocol */
        8,                      /* max packet size */
        0x0D, 0x0F,             //        USB_CFG_VENDOR_ID,	/* 2 bytes */0x0F0D
        0x92, 0x00,             //USB_CFG_DEVICE_ID,    /* 2 bytes */ 0x0092
        USB_CFG_DEVICE_VERSION, /* 2 bytes */
        1,                      /* manufacturer string index */
        2,                      /* product string index */
        0,                      /* serial number string index */
        1,                      /* number of configurations */
};

const PROGMEM char joyconDescriptorConfiguration[] = {    /* USB configuration descriptor */
        9,          /* sizeof(usbDescriptorConfiguration): length of descriptor in bytes */
        USBDESCR_CONFIG,    /* descriptor type */
        18 + 7 * USB_CFG_HAVE_INTRIN_ENDPOINT + 7 * USB_CFG_HAVE_INTRIN_ENDPOINT3 + 9, 0,
        /* total length of data returned (including inlined descriptors) */
        1,          /* number of interfaces in this configuration */
        1,          /* index of this configuration */
        0,          /* configuration name string index */
        0x80,                           /* attributes */
        500 >> 1,            /* max USB current in 2mA units */
/* interface descriptor follows inline: */
        9,          /* sizeof(usbDescrInterface): length of descriptor in bytes */
        USBDESCR_INTERFACE, /* descriptor type */
        0,          /* index of this interface */
        0,          /* alternate setting for this interface */
        2,//USB_CFG_HAVE_INTRIN_ENDPOINT + USB_CFG_HAVE_INTRIN_ENDPOINT3, /* endpoints excl 0: number of endpoint descriptors to follow */
        USB_CFG_INTERFACE_CLASS,
        USB_CFG_INTERFACE_SUBCLASS,
        USB_CFG_INTERFACE_PROTOCOL,
        0,          /* string index for interface */
        //USB_CFG_DESCR_PROPS_HID
        9,          /* sizeof(usbDescrHID): length of descriptor in bytes */
        USBDESCR_HID,   /* descriptor type: HID */
        0x11, 0x01, /* BCD representation of HID version */
        0x00,       /* target country code */
        0x01,       /* number of HID Report (or other HID class) Descriptor infos to follow */
        USBDESCR_HID_REPORT,       /* descriptor type: report */
        USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH, 0,  /* total length of report descriptor */
#if USB_CFG_HAVE_INTRIN_ENDPOINT   /* endpoint descriptor for endpoint 3 */
        7,          /* sizeof(usbDescrEndpoint) */
        USBDESCR_ENDPOINT,  /* descriptor type = endpoint */
        0x81,//ENDPOINT_DIR_IN|1,//2,//(char) 0x83, /* IN endpoint number 1 */
        0x03,       /* attrib: Interrupt endpoint */
        64, 0,       /* maximum packet size */
        0x05,//USB_CFG_INTR_POLL_INTERVAL, /* in ms */
#endif
#if USB_CFG_HAVE_INTRIN_ENDPOINT3   /* endpoint descriptor for endpoint 3 */
        7,          /* sizeof(usbDescrEndpoint) */
        USBDESCR_ENDPOINT,  /* descriptor type = endpoint */
        2,//ENDPOINT_DIR_OUT | 2,//2,//(char) 0x83, /* IN endpoint number 1 */
        0x03,       /* attrib: Interrupt endpoint */
        64, 0,       /* maximum packet size */
        0x05,//USB_CFG_INTR_POLL_INTERVAL, /* in ms */
#endif
};

#define GCN64_REPORT_SIZE 8
#define joycon_DEFAULT_REPORT_INTERVAL 20

static uchar reportBuffer[8];
static const uchar *rt_usbHidReportDescriptor = NULL;
static const uchar *rt_usbDeviceDescriptor = NULL;
static uchar rt_usbHidReportDescriptorSize = 0;
static uchar rt_usbDeviceDescriptorSize = 0;
static unsigned char last_built_report[GCN64_REPORT_SIZE];
static unsigned char last_sent_report[GCN64_REPORT_SIZE];
static unsigned char idle_rate = joycon_DEFAULT_REPORT_INTERVAL / 4; // in units of 4ms
static unsigned char must_report = 0;
static unsigned long last_report_time = 0;

static void gamecubeBuildReport(unsigned char *reportBuf) {
    if (reportBuf != NULL) {
        memcpy(reportBuf, last_built_report, GCN64_REPORT_SIZE);
    }
    memcpy(last_sent_report, last_built_report, GCN64_REPORT_SIZE);
}

static int getGamepadReport(unsigned char *dstbuf) {
    gamecubeBuildReport(dstbuf);
    return GCN64_REPORT_SIZE;
}

uchar usbFunctionSetup(uchar data[8]) {
    usbRequest_t *rq = (usbRequest_t *) data;
    usbMsgPtr = reportBuffer;
    if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) { // class request type
        if (rq->bRequest == USBRQ_HID_GET_REPORT) {
            return GCN64_REPORT_SIZE;
        } else if (rq->bRequest == USBRQ_HID_GET_IDLE) {
            usbMsgPtr = &idle_rate;
            return 1;
        } else if (rq->bRequest == USBRQ_HID_SET_IDLE) {
            idle_rate = rq->wValue.bytes[1];
        }
    } else {

    }
    return 0;
}

#define JOYCON_VENDOR_NAME     'H','O','R','I',' ','C','O','.',',','L','T','D'
#define JOYCON_VENDOR_NAME_LEN 12
const PROGMEM int joyconDescriptorStringVendor[] = {
        USB_STRING_DESCRIPTOR_HEADER(JOYCON_VENDOR_NAME_LEN),
        JOYCON_VENDOR_NAME
};
#define JOYCON_DEVICE_NAME     'P','O','K','K','E','N',' ','C','O','N','T','R','O','L','L','E','R'
#define JOYCON_DEVICE_NAME_LEN 17
const PROGMEM int joyconDescriptorStringDevice[] = {
        USB_STRING_DESCRIPTOR_HEADER(JOYCON_DEVICE_NAME_LEN),
        JOYCON_DEVICE_NAME
};

uchar usbFunctionDescriptor(struct usbRequest *rq) {
    if ((rq->bmRequestType & USBRQ_TYPE_MASK) != USBRQ_TYPE_STANDARD)
        return 0;
    if (rq->bRequest == USBRQ_GET_DESCRIPTOR) {
        switch (rq->wValue.bytes[1]) {
            case USBDESCR_DEVICE:
                usbMsgPtr = rt_usbDeviceDescriptor;
                return rt_usbDeviceDescriptorSize;
            case USBDESCR_CONFIG:
                usbMsgPtr = (uchar *) (joyconDescriptorConfiguration);
                return USB_PROP_LENGTH(sizeof(joyconDescriptorConfiguration));
            case USBDESCR_STRING:
                switch (rq->wValue.bytes[0]) {
                    case 1:
                        usbMsgPtr = (uchar *) (joyconDescriptorStringVendor);
                        return USB_PROP_LENGTH(sizeof(joyconDescriptorStringVendor));
                    case 2:
                        usbMsgPtr = (uchar *) (joyconDescriptorStringDevice);
                        return USB_PROP_LENGTH(sizeof(joyconDescriptorStringDevice));
                }
                break;
            case USBDESCR_HID:
                usbMsgPtr = (uchar *) (joyconDescriptorConfiguration + 18);
                return USB_PROP_LENGTH(9);
            case USBDESCR_HID_REPORT:
                usbMsgPtr = rt_usbHidReportDescriptor;
                return rt_usbHidReportDescriptorSize;
        }
    }
    return 0;
}

// implements

void joycon_init() {
    cli();
    usbDeviceDisconnect();
    _delay_ms(250);
    usbDeviceConnect();
    rt_usbHidReportDescriptor = gcn64_usbHidReportDescriptor;
    rt_usbHidReportDescriptorSize = sizeof(gcn64_usbHidReportDescriptor);
    rt_usbDeviceDescriptor = usbDescrDevice;
    rt_usbDeviceDescriptorSize = sizeof(usbDescrDevice);
    usbInit();
    sei();
    last_report_time = millis();
}

void joycon_update() {
    usbPoll();
    unsigned long time_since_last_report = millis() - last_report_time;
    if (time_since_last_report >= (idle_rate * 4 /* in units of 4ms - usb spec stuff */)) {
        last_report_time += idle_rate * 4;
        must_report = 1;
    }
    // if the report has changed, try force an update anyway
    if (memcmp(last_built_report, last_sent_report, GCN64_REPORT_SIZE)) {
        must_report = 1;
    }
    // if we want to send a report, signal the host computer to ask us for it with a usb 'interrupt'
    if (must_report) {
        if (usbInterruptIsReady()) {
            must_report = 0;

            gamecubeBuildReport(reportBuffer);
            usbSetInterrupt(reportBuffer, GCN64_REPORT_SIZE);
        }
    }
}

void joycon_delay(long milli) {
    unsigned long last = millis();
    while (milli > 0) {
        unsigned long now = millis();
        milli -= now - last;
        last = now;
        joycon_update();
    }
}

void joycon_setX(uint8_t value) {
    last_built_report[0] = value;
}

void joycon_setY(uint8_t value) {
    last_built_report[1] = value;
}

void joycon_setXROT(uint8_t value) {
    last_built_report[2] = value;
}

void joycon_setYROT(uint8_t value) {
    last_built_report[3] = value;
}

void joycon_setZROT(uint8_t value) {
    last_built_report[4] = value;
}

void joycon_setSLIDER(uint8_t value) {
    last_built_report[5] = value;
}


void joycon_setButtons(uint8_t low, uint8_t high) {
    last_built_report[6] = low;
    last_built_report[7] = high;
}

void joycon_setValues(uint8_t values[]) {
    memcpy(last_built_report, values, GCN64_REPORT_SIZE);
}