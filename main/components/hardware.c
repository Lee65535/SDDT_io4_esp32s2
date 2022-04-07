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
};

// 1 if is always-open switch
const uint8_t PINS_MODES[] = {
        1, 1, 1, 0, 1,
        1, 1, 1, 0, 1,
};

// see define of adc1_channel_t, the number below is not pin
const adc1_channel_t LEVER_PIN = ADC1_CHANNEL_0;

void hardware_init() {
    for(int i = 0; i < 10; i++) {
        gpio_set_direction(PIN_SETTINGS[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(PIN_SETTINGS[i], GPIO_PULLUP_ONLY);
    }

    adc1_config_width(ADC_WIDTH_BIT_13);
    adc1_config_channel_atten(LEVER_PIN, ADC_ATTEN_DB_11);
}

bool hardware_update(hid_output_t *output) {
    bool modified = false;

    // TODO modify to IO4 format

    for(int i = 0; i < 10; i++) {
        int state = gpio_get_level(PIN_SETTINGS[i]);
        if(PINS_MODES[i]) {
            state = !state;
        }

        if(output->buttons[i] != state)
        {
            modified = true;
            output->buttons[i] = state;
        }
    }

    static float lever_smooth = 0;
    lever_smooth = lever_smooth * 0.5f + (float)adc1_get_raw(LEVER_PIN) * 0.5f;

    output->lever = (int16_t)lever_smooth;
    return modified;
}

