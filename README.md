# ESP32-S2 Ongeki IO4

Forked from GEEKiDoS's esp32-s2 ongeki con

HUGE thank to GEEKiDoS, I (Lee) did nothing but write this README

**esp-idf version: 4.4**

## Setup

Before proceeding, make sure you have esp-idf 4.4 installed;

For CLion, you can set environment variables in settings/preferences without altering PATH system-wise.

In `idf.py menuconfig`, enable TinyUSB stack, enable HID and modify configurations accordingly.

### esp-idf Files Need to Be Modified:

All files below are in your esp-idf folder (typically C:\Users\<you>\esp-idf\ if Windows)

- components/tinyusb/Kconfig
- components/tinyusb/additions/include/tusb_config.h

(above files are attached in "modify idf/", just copy and paste)

- components/tinyusb/addition/src/descriptors_control.c and .h (modify according to guidance below)

### USB Config

#### In `idf.py menuconfig`

Device Name: `I/O CONTROL BD;15257;01;90;1831;6679A;00;GOUT=14_ADIN=8,E_ROTIN=4_COININ=2_SWIN=2,E_UQ1=41,6;`

Vendor ID: `0x0CA3`

Product ID: `0x0021` (does not matter)

Manufacturer: `SEGA` (does not matter)

#### In `descriptors_control.c`

1. Change `#define MAX_DESC_BUF_SIZE 32` to `#define MAX_DESC_BUF_SIZE 128`
2. Change `TUD_HID_DESCRIPTOR` line to

```c++
TUD_HID_INOUT_DESCRIPTOR(ITF_NUM_HID, 0, HID_PROTOCOL_NONE, sizeof(desc_hid_report), 0x01,0x81, CFG_TUD_HID_EP_BUFSIZE, 1)
```

3. Change `desc_hid_report` to

```C++
uint8_t const desc_hid_report[] = {
        0x05, 0x01,                     // Usage Page (Generic Desktop Ctrls)
        0x09, 0x04,                     // Usage (Joystick)
        0xA1, 0x01,                     // Collection (Application)
        0x85, 0x01,                     //   Report ID (1)
        0x09, 0x01,                     //   Usage (Pointer)
        0xA1, 0x00,                     //   Collection (Physical)
        0x09, 0x30,                     //     Usage (X)
        0x09, 0x31,                     //     Usage (Y)
        0x09, 0x30,                     //     Usage (X)
        0x09, 0x31,                     //     Usage (Y)
        0x09, 0x30,                     //     Usage (X)
        0x09, 0x31,                     //     Usage (Y)
        0x09, 0x30,                     //     Usage (X)
        0x09, 0x31,                     //     Usage (Y)
        0x09, 0x33,                     //     Usage (Rx)
        0x09, 0x34,                     //     Usage (Ry)
        0x09, 0x33,                     //     Usage (Rx)
        0x09, 0x34,                     //     Usage (Ry)
        0x09, 0x36,                     //     Usage (Slider)
        0x09, 0x36,                     //     Usage (Slider)
        0x15, 0x00,                     //     Logical Minimum (0)
        0x27, 0xFF, 0xFF, 0x00, 0x00,   //     Logical Maximum (65534)
        0x35, 0x00,                     //     Physical Minimum (0)
        0x47, 0xFF, 0xFF, 0x00, 0x00,   //     Physical Maximum (65534)
        0x95, 0x0E,                     //     Report Count (14)
        0x75, 0x10,                     //     Report Size (16)
        0x81, 0x02,                     //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0xC0,                           //   End Collection
        0x05, 0x02,                     //   Usage Page (Sim Ctrls)
        0x05, 0x09,                     //   Usage Page (Button)
        0x19, 0x01,                     //   Usage Minimum (0x01)
        0x29, 0x30,                     //   Usage Maximum (0x30)
        0x15, 0x00,                     //   Logical Minimum (0)
        0x25, 0x01,                     //   Logical Maximum (1)
        0x45, 0x01,                     //   Physical Maximum (1)
        0x75, 0x01,                     //   Report Size (1)
        0x95, 0x30,                     //   Report Count (48)
        0x81, 0x02,                     //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x09, 0x00,                     //   Usage (0x00)
        0x75, 0x08,                     //   Report Size (8)
        0x95, 0x1D,                     //   Report Count (29)
        0x81, 0x01,                     //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x06, 0xA0, 0xFF,               //   Usage Page (Vendor Defined 0xFFA0)
        0x09, 0x00,                     //   Usage (0x00)
        0x85, 0x10,                     //   Report ID (16)
        0xA1, 0x01,                     //   Collection (Application)
        0x09, 0x00,                     //     Usage (0x00)
        0x15, 0x00,                     //     Logical Minimum (0)
        0x26, 0xFF, 0x00,               //     Logical Maximum (255)
        0x75, 0x08,                     //     Report Size (8)
        0x95, 0x3F,                     //     Report Count (63)
        0x91, 0x02,                     //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0xC0,                           //   End Collection
        0xC0,                           // End Collection
};
```

#### In descriptors_control.h

Change `TUSB_DESC_TOTAL_LEN` to

```c++
TUSB_DESC_TOTAL_LEN = TUD_CONFIG_DESC_LEN + CFG_TUD_CDC * TUD_CDC_DESC_LEN + CFG_TUD_MSC * TUD_MSC_DESC_LEN + CFG_TUD_HID * TUD_HID_INOUT_DESC_LEN
```
