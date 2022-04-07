#include <driver/gpio.h>
#include <driver/adc_common.h>
#include <sys/param.h>

#include "usb_hid.h"
#include "hardware.h"

const gpio_num_t PIN_SETTINGS[] = {
        // LA LB LC LS LM
        GPIO_NUM_33, GPIO_NUM_34, GPIO_NUM_35, GPIO_NUM_36, GPIO_NUM_37,
        // RA RB RC RS RM
        GPIO_NUM_38, GPIO_NUM_39, GPIO_NUM_40, GPIO_NUM_41, GPIO_NUM_42,
        // HELLO(TEST&SERVICE)
        GPIO_NUM_43
};

// 0 for NC switch (normal microswitch), 1 for NO switch (side button)
const uint8_t PINS_MODES[] = {
        1, 1, 1, 0, 1,
        1, 1, 1, 0, 1,
};

// see definition of adc1_channel_t; the number below is not pin
const adc1_channel_t LEVER_PIN = ADC1_CHANNEL_0;

void hardware_init() {
    for (int i = 0; i < 10; i++) {
        gpio_set_direction(PIN_SETTINGS[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(PIN_SETTINGS[i], GPIO_PULLUP_ONLY);
    }

    adc1_config_width(ADC_WIDTH_BIT_13);
    adc1_config_channel_atten(LEVER_PIN, ADC_ATTEN_DB_11);
}

uint8_t io4_system_status;

void hardware_update(io4_output_t *data) {
    // for position in data->Switch, ref to segatools
    uint16_t PIN_MAP[] = {
            LA, LB, LC, LS, LM,
            RA, RB, RC, RS, RM,
    };

    if (gpio_get_level(PIN_SETTINGS[10])) {
        data->Switch[0] |= TEST; // TEST
        data->Switch[0] |= SERVICE; // SERVICE
    }

    for (int i = 0; i < 10; i++) {
        int state = gpio_get_level(PIN_SETTINGS[i]);
        if (state) {
            if (i == 6 || i == 4 || i == 3) {
                data->Switch[1] |= PIN_MAP[i];
            } else {
                data->Switch[0] |= PIN_MAP[i];
            }
        }
    }

    static float lever_smooth = 0;
    lever_smooth = lever_smooth * 0.5f + (float) adc1_get_raw(LEVER_PIN) * 0.5f;
    data->Analog[0] = (int16_t) (0x7FFF - lever_smooth);
    data->report_id = 0x01;
    data->SystemStatus = io4_system_status;
    data->UsbStatus = 0;
}

void input_callback(io4_input_t * input) {
    switch (input->cmd) {
        case IO4_CMD_SET_COMM_TIMEOUT:
            io4_system_status = AM_USBIOP_SYSTEM_STATUS_TIMEOUT_SETTING;
            break;
        case IO4_CMD_SET_SAMPLING_COUNT:
            io4_system_status = AM_USBIOP_SYSTEM_STATUS_SAMPLING_SETTING;
            break;
        case IO4_CMD_CLEAR_BOARD_STATUS:
            io4_system_status = 0x00;
            break;
        default:
            io4_system_status = 0x00;
            break;
    }
}

