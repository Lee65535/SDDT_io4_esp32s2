#pragma once

#pragma pack(push, 1)
typedef struct {
    uint8_t buttons[10];
    int16_t lever;
    uint8_t scan;
    uint8_t aimi_id[10];
    char _shortpad[41];
} hid_output_t;
typedef struct AM_USBIOP_INPUT_DATA {
    uint8_t report_id;
    int16_t Analog[8]; // adcs
    int16_t Rotary[4]; // spinners
    int16_t Coin[2]; // chutes
    uint16_t Switch[2]; // buttons
    uint8_t SystemStatus;
    uint8_t UsbStatus;
    uint8_t ExInput[29];
} __attribute((packed)) io4_output_t;
typedef struct {
    uint8_t report_id;
    uint8_t cmd;
    uint8_t payload[62];
} io4_input_t;

enum USBStatus {
    IO4_CMD_SET_COMM_TIMEOUT = 0x01,
    IO4_CMD_SET_SAMPLING_COUNT = 0x02,
    IO4_CMD_CLEAR_BOARD_STATUS = 0x03,
    IO4_CMD_SET_GENERAL_OUTPUT = 0x04,
    IO4_CMD_SET_PWM_OUTPUT = 0x05,
    IO4_CMD_UPDATE_FIRMWARE = 0x85,
};

enum SystemStatus {
    AM_USBIOP_SYSTEM_STATUS_POWER_ON_RESET = 0x1,
    AM_USBIOP_SYSTEM_STATUS_SYSTEM_RESET = 0x2,
    AM_USBIOP_SYSTEM_STATUS_WATCHDOG_RESET = 0x4,
    AM_USBIOP_SYSTEM_STATUS_BROWNOUT_RESET = 0x8,
    AM_USBIOP_SYSTEM_STATUS_TIMEOUT_SETTING = 0x10,
    AM_USBIOP_SYSTEM_STATUS_SAMPLING_SETTING = 0x20,
};

#pragma pack(pop)

void usb_init();

void usb_hid_report(io4_output_t *);
