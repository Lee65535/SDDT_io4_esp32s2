//
// Created by GEEKiDoS on 2021/10/29.
//
#include <string.h>
#include <sys/param.h>
#include "driver/uart.h"
#include "driver/gpio.h"
#include "card_reader.h"

const int uart_port = UART_NUM_1;
uint64_t felica_id = 0;

void card_reader_init() {
    uart_config_t uartConfig = {
            .baud_rate = 19200,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    uart_driver_install(uart_port, 512, 512, 10, NULL, 0);
    uart_param_config(uart_port, &uartConfig);
    uart_set_pin(uart_port, GPIO_NUM_16, GPIO_NUM_17, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

_Noreturn void card_reader_task(void *param) {
    (void) param;

    cu500_frame_header_t *command = (cu500_frame_header_t *) malloc(5);
    command->length = 5;
    command->module_address = 1;
    // activate other type card
    command->command = 0x17;
    // card type, 0x01 = felica
    command->data[0] = 0x01;
    // checksum
    command->data[1] = cu500_calc_checksum(command);

    cu500_frame_header_t *result = (cu500_frame_header_t *) malloc(0x40);

    for (;;) {
        memset(result, 0, 0x40);

        uart_write_bytes(uart_port, command, 5);
        uart_read_bytes(uart_port, result, 1, 100);

        if (result->length > 0) {
            uart_read_bytes(uart_port, result + 1, MIN(0x40, result->length) - 1, 100);

            // check if success
            if (result->length == 0x0D) {
                felica_id = *((uint64_t *) (result->data));
            } else {
                felica_id = 0;
            }
        } else {
            felica_id = 0;
        }

        // card reader don't need to be fast
        vTaskDelay(300 / portTICK_RATE_MS);
    }
}

uint64_t get_felica_id() {
    return felica_id;
}
