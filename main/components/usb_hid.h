#pragma once

#pragma pack(push, 1)
typedef struct {
    uint8_t buttons[10];
    int16_t lever;
    uint8_t scan;
    uint8_t aimi_id[10];
    char _shortpad[41];
} hid_output_t;
#pragma pack(pop)

void usb_init();
void usb_hid_report(hid_output_t *);
