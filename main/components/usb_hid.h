#pragma once

#pragma pack(push, 1)
// don't be confused here
// output_t is io4->SDDT
typedef struct AM_USBIOP_INPUT_DATA {
    int16_t Analog[8]; // adcs
    int16_t Rotary[4]; // spinners
    uint16_t Coin[2]; // chutes
    uint16_t Switch[2]; // buttons
    uint8_t SystemStatus;
    uint8_t UsbStatus;
    uint8_t ExInput[29];
} __attribute((packed)) io4_output_t;
typedef struct AM_USBIOP_OUTPUT_DATA {
    uint8_t report_id;
    uint8_t cmd;
    uint8_t payload[62];
} __attribute((packed)) io4_input_t;

enum USBStatus {
    IO4_CMD_SET_COMM_TIMEOUT = 0x01,
    IO4_CMD_SET_SAMPLING_COUNT = 0x02,
    IO4_CMD_CLEAR_BOARD_STATUS = 0x03,
    IO4_CMD_SET_GENERAL_OUTPUT = 0x04,
    IO4_CMD_SET_PWM_OUTPUT = 0x05,
    IO4_CMD_UPDATE_FIRMWARE = 0x85,
};

#pragma pack(pop)

void usb_init();

void usb_hid_report(io4_output_t *);
