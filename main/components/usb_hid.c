#include <sys/cdefs.h>
#include <sys/param.h>

#include "tinyusb.h"

#include "usb_hid.h"
#include "hardware.h"
#include "ws2812.h"


void usb_init() {
    tinyusb_config_t tusb_cfg = {};
    tinyusb_driver_install(&tusb_cfg);
}

void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t buffer_size) {
    io4_input_t* input = (io4_input_t* )buffer;

    if (input->report_id == 0x10) {
        input_callback(input);
    }

}

uint16_t
tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    return 0;
}

void usb_hid_report(io4_output_t *data) {
    if(tud_hid_ready()) {
        tud_hid_report(1, data, sizeof(io4_output_t));
    }
}
