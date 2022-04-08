#include <string.h>
#include <sys/param.h>
#include "driver/uart.h"
#include "driver/gpio.h"
#include "card_reader.h"
#include "pn532.h"

const int uart_port = UART_NUM_1;

void card_reader_init() {
    pn532_begin(uart_port);
    uint32_t versiondata = pn532_get_firmware_version();
    printf("Found chip PN5");
    uint8_t buffer[] = {(versiondata >> 24)};
    PrintHexChar(buffer, 1);
    pn532_set_passive_activation_retries(0xFF);
    pn532_SAM_config();
    xTaskCreate(card_reader_task, "card_reader", 4096, NULL, 1, NULL);
}

_Noreturn void card_reader_task(void *param) {
    (void) param;

    for (;;) {



        // card reader don't need to be fast
        vTaskDelay(300 / portTICK_RATE_MS);
    }
}