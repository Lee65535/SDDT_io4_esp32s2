#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "components/card_reader.h"
#include "components/usb_hid.h"
#include "components/hardware.h"
#include "components/ws2812.h"

_Noreturn void app_main(void) {
    printf("-------- board init --------\n");
    printf("init usb hid device\n");
    usb_init();
    printf("init hardware\n");
    hardware_init();
    printf("init led\n");
    led_init();
    printf("-------- done --------\n");

    io4_output_t data;
    memset(&data, 0, 64);

    for(;;) {
        vTaskDelay(2 / portTICK_RATE_MS);
    }
}
