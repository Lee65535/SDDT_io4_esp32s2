#include <sys/cdefs.h>
#include <sys/param.h>

#include "tinyusb.h"

#include "usb_hid.h"
#include "ws2812.h"


void usb_init() {
    tinyusb_config_t tusb_cfg = {};
    tinyusb_driver_install(&tusb_cfg);
}

void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t buffer_size) {
    led_update((color_t *)buffer);
}

uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    return 0;
}

void usb_hid_report(hid_output_t *data) {
    if(tud_hid_ready()) {
        tud_hid_report(0, data, sizeof(hid_output_t));
    }
}
