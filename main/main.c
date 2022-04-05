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
    printf("init card reader\n");
    card_reader_init();
    xTaskCreate(card_reader_task, "card_reader", 4096, NULL, 1, NULL);
    printf("init usb hid device\n");
    usb_init();
    printf("init hardware\n");
    hardware_init();
    printf("init led\n");
    led_init();
    printf("-------- done --------\n");

    char card[21];
    hid_output_t data;
    memset(&data, 0, 64);

    uint64_t id;

    for(;;) {
        bool send = hardware_update(&data);
        id = get_felica_id();
        if(!data.scan && id) {
            // convert to hex represented dec
            sprintf(card, "%020llu", id);
            printf("%s\n", card);
            for (int i = 0; i < 10; i++) {
                data.aimi_id[i] = ((card[i * 2] - '0') << 4) | (card[i * 2 + 1] - '0');
            }

            data.scan = true;
            send = true;
        } else if(data.scan && !id) {
            memset(data.aimi_id, 0, 10);
            data.scan = false;
            send = true;
        }

        usb_hid_report(&data);
        vTaskDelay(2 / portTICK_RATE_MS);
    }
}
