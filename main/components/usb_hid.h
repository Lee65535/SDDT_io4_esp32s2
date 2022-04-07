#pragma once

#pragma pack(push, 1)
typedef struct {
    uint8_t buttons[10];
    int16_t lever;
    uint8_t scan;
    uint8_t aimi_id[10];
    char _shortpad[41];
} hid_output_t;
typedef struct AM_USBIOP_INPUT_DATA
{
    uint8_t ReportId;
    int16_t Analog[8];
    int16_t Rotary[4];
    int16_t Coin[2];
    uint16_t Switch[2];
    uint8_t SystemStatus;
    uint8_t UsbStatus;
    uint8_t ExInput[29];
} __attribute((packed)) io4_output_t;
#pragma pack(pop)

void usb_init();
void usb_hid_report(hid_output_t *);
