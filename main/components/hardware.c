#include <driver/gpio.h>
#include <driver/adc_common.h>

#include "usb_hid.h"
#include "hardware.h"

const gpio_num_t PIN_SETTINGS[] = {
        // LA LB LC LS LM
        GPIO_NUM_33, GPIO_NUM_34, GPIO_NUM_35, GPIO_NUM_36, GPIO_NUM_37,
        // RA RB RC RS RM
        GPIO_NUM_38, GPIO_NUM_39, GPIO_NUM_40, GPIO_NUM_41, GPIO_NUM_42,
        // HELLO(TEST&SERVICE)
        GPIO_NUM_45,
        // COIN (WIP)
        GPIO_NUM_46
};

// 0 for NC switch (normal microswitch), 1 for NO switch (side button)
const uint8_t PINS_MODES[] = {
        1, 1, 1, 0, 1,
        1, 1, 1, 0, 1,
};

// see definition of adc1_channel_t; the number below is not pin
const adc1_channel_t LEVER_PIN = ADC1_CHANNEL_0;

void hardware_init() {
    for (int i = 0; i < 12; i++) {
        gpio_set_direction(PIN_SETTINGS[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(PIN_SETTINGS[i], GPIO_PULLUP_ONLY);
    }

    adc1_config_width(ADC_WIDTH_BIT_13);
    adc1_config_channel_atten(LEVER_PIN, ADC_ATTEN_DB_11);
}

uint8_t io4_system_status = 0x02;

uint8_t coin_prev_state = 0;
uint16_t coin_n = 0;

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
        if (state ^ PINS_MODES[i]) {
            if (i == 6 || i == 4 || i == 3) {
                data->Switch[1] |= PIN_MAP[i];
            } else {
                data->Switch[0] |= PIN_MAP[i];
            }
        }
    }

    // WIP&untested: Coin
    uint8_t coin_state = gpio_get_level(PIN_SETTINGS[11]);
    if (coin_state && !coin_prev_state) {
        coin_n++;
    }
    coin_prev_state = coin_state;
    data->Coin[0] = coin_n;
    data->Coin[1] = coin_n;

    static float lever_smooth = 0;
    lever_smooth = lever_smooth * 0.5f + (float) adc1_get_raw(LEVER_PIN) * 0.5f;
    data->Analog[0] = (int16_t) (0x7FFF - lever_smooth);
    data->SystemStatus = io4_system_status;
    data->UsbStatus = 0;
}

void input_callback(io4_input_t *input) {
    switch (input->cmd) {
        case IO4_CMD_SET_COMM_TIMEOUT:
            printf("IO4: SET COMMUNICATE TIMEOUT\n");
            io4_system_status = 0x30;
            break;
        case IO4_CMD_SET_SAMPLING_COUNT:
            printf("IO4: SET SAMPLING COUNT\n");
            io4_system_status = 0x30;
            break;
        case IO4_CMD_CLEAR_BOARD_STATUS:
            printf("IO4: CLEAR BOARD STATUS\n");
            io4_system_status = 0x00;
            break;
        case IO4_CMD_SET_GENERAL_OUTPUT:
            printf("IO4: SET GENERAL OUTPUT\n");

            break;
        default:
            break;
    }
}
