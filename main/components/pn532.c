/*
 * Basically copied from SeeedStuidio/PN532
 * Physical Interface: HSU
 */
#include <string.h>
#include <time.h>
#include <tinyusb.h>
#include <sys/param.h>
#include <driver/uart.h>
#include <driver/gpio.h>
#include "pn532.h"

uint8_t command;
uint8_t pn532_packetbuffer[64];
uint8_t pn532_uid[7];  // ISO14443A uid
uint8_t pn532_uidLen;  // uid len
uint8_t pn532_key[6];  // Mifare Classic key
uint8_t inListedTag;
uint8_t pn532_felicaIDm[8]; // FeliCa IDm (NFCID2)
uint8_t pn532_felicaPMm[8]; // FeliCa PMm (PAD)
int uart_port;

void pn532_begin(const int _uart_port) {
    // start UART
    uart_port = _uart_port;
    uart_config_t uartConfig = {
            .baud_rate = 115200,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_driver_install(uart_port, 512, 512, 10, NULL, 0);
    uart_param_config(uart_port, &uartConfig);
    uart_set_pin(uart_port, GPIO_NUM_16, GPIO_NUM_17, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    // wakeup PN532
    const uint8_t wakeup_command[] = {0x55, 0x55, 0x00, 0x00, 0x00};
    uart_write_bytes(uart_port, wakeup_command, sizeof(wakeup_command));
    uint8_t buffer[1];
    while (uart_read_bytes(uart_port, buffer, 1, 100) > 0) {
        // print some debug info
    }
}

int8_t pn532_write_command_full(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen) {
    // dump serial buffer
    uint8_t buffer[3];
    while (uart_read_bytes(uart_port, buffer, 1, 100) > 0) {
        // print some debug info
    }
    command = header[0];
    buffer[0] = PN532_PREAMBLE;
    buffer[1] = PN532_STARTCODE1;
    buffer[2] = PN532_STARTCODE2;
    uart_write_bytes(uart_port, buffer, 3);
    uint8_t length = hlen + blen + 1; // length of data field: TFI + DATA
    buffer[0] = length;
    buffer[1] = ~length + 1; // checksum
    buffer[2] = PN532_HOSTTOPN532;
    uart_write_bytes(uart_port, buffer, 3);
    uint8_t sum = PN532_HOSTTOPN532; // sum of TFI + DATA
    uart_write_bytes(uart_port, header, hlen);
    for (int i = 0; i < hlen; ++i) {
        sum += header[i];
    }
    uart_write_bytes(uart_port, body, blen);
    for (int i = 0; i < blen; ++i) {
        sum += body[i];
    }
    uint8_t checksum = ~sum + 1;
    buffer[0] = checksum;
    buffer[1] = PN532_POSTAMBLE;
    uart_write_bytes(uart_port, buffer, 2);

    return pn532_read_ack_frame();
}

int8_t pn532_write_command(const uint8_t *header, uint8_t hlen) {
    return pn532_write_command_full(header, hlen, 0, 0);
}

int8_t pn532_read_ack_frame() {
    const uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};
    uint8_t ackBuf[sizeof(PN532_ACK)];
    if (pn532_receive(ackBuf, sizeof(PN532_ACK), PN532_ACK_WAIT_TIME) <= 0) {
        return PN532_TIMEOUT;
    }
    if (memcmp(ackBuf, PN532_ACK, sizeof(PN532_ACK)) != 0) {
        return PN532_INVALID_ACK;
    }
    return 0;
}

int16_t pn532_read_response_full(uint8_t buf[], uint8_t len, uint16_t timeout) {
    uint8_t tmp[3];
    /** Frame Preamble and Start Code */
    if (pn532_receive(tmp, 3, timeout) <= 0) {
        return PN532_TIMEOUT;
    }
    if (0 != tmp[0] || 0 != tmp[1] || 0xFF != tmp[2]) {
        //Preamble error
        return PN532_INVALID_FRAME;
    }

    /** receive length and check */
    uint8_t length[2];
    if (pn532_receive(length, 2, timeout) <= 0) {
        return PN532_TIMEOUT;
    }
    if (0 != (uint8_t) (length[0] + length[1])) {
        // Length error
        return PN532_INVALID_FRAME;
    }
    length[0] -= 2;
    if (length[0] > len) {
        return PN532_NO_SPACE;
    }

    /** receive command byte */
    uint8_t cmd = command + 1; // response command
    if (pn532_receive(tmp, 2, timeout) <= 0) {
        return PN532_TIMEOUT;
    }
    if (PN532_PN532TOHOST != tmp[0] || cmd != tmp[1]) {
        // Command error
        return PN532_INVALID_FRAME;
    }

    if (pn532_receive(buf, length[0], timeout) != length[0]) {
        return PN532_TIMEOUT;
    }
    uint8_t sum = PN532_PN532TOHOST + cmd;
    for (uint8_t i = 0; i < length[0]; i++) {
        sum += buf[i];
    }

    /** checksum and postamble */
    if (pn532_receive(tmp, 2, timeout) <= 0) {
        return PN532_TIMEOUT;
    }
    if (0 != (uint8_t) (sum + tmp[0]) || 0 != tmp[1]) {
        // Checksum error
        return PN532_INVALID_FRAME;
    }

    return length[0];
}

int16_t pn532_read_response(uint8_t buf[], uint8_t len) {
    return pn532_read_response_full(buf, len, 500);
}

int8_t pn532_receive(uint8_t *buf, int len, uint16_t timeout) {
    int8_t read_bytes = 0;
    int ret;
    clock_t start;
    while (read_bytes < len) {
        start = clock();
        do {
            uart_read_bytes(uart_port, &ret, 1, 10);
            if (ret >= 0) break;
        } while ((timeout == 0) || ((clock() - start) < timeout));

        if (ret < 0) {
            if (read_bytes) {
                return read_bytes;
            } else {
                return PN532_TIMEOUT;
            }
            buf[read_bytes] = (uint8_t) ret;
            read_bytes++;
        }
        buf[read_bytes] = (uint8_t) ret;
        read_bytes++;
    }
    return read_bytes;
}

// HSU Abstraction Layer
// ##############################
// PN532 Command Abstraction

void PrintHexChar(const uint8_t *data, const uint32_t numBytes) {
    for (uint8_t i = 0; i < numBytes; i++) {
        printf(" %2X", data[i]);
    }
    printf("    ");
    for (uint8_t i = 0; i < numBytes; i++) {
        char c = (char) data[i];
        if (c <= 0x1f || c > 0x7f) {
            printf(".");
        } else {
            printf("%c", c);
        }
        printf("\n");
    }
}

uint32_t pn532_get_firmware_version() {
    uint32_t response;
    pn532_packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;
    if (pn532_write_command(pn532_packetbuffer, 1)) return 0;
    // read data packet
    int16_t status = pn532_read_response(pn532_packetbuffer, sizeof(pn532_packetbuffer));
    if (0 > status) return 0;
    response = pn532_packetbuffer[0];
    response <<= 8;
    response |= pn532_packetbuffer[1];
    response <<= 8;
    response |= pn532_packetbuffer[2];
    response <<= 8;
    response |= pn532_packetbuffer[3];

    return response;
}

bool pn532_set_passive_activation_retries(uint8_t max_retries) {
    uint8_t buffer[] = {
            PN532_COMMAND_RFCONFIGURATION,
            0x05, // Config item 5 (MaxRetries)
            0xFF, // MxRtyATR (default = 0xFF)
            0x01, // MxRtyPSL (default = 0x01)
            max_retries
    };
    if (pn532_write_command(buffer, 5)) return 0x0; // no ACK
    return 0 < pn532_read_response(pn532_packetbuffer, sizeof(pn532_packetbuffer));
}

bool pn532_SAM_config() {
    pn532_packetbuffer[0] = PN532_COMMAND_SAMCONFIGURATION;
    pn532_packetbuffer[1] = 0x01; // normal mode;
    pn532_packetbuffer[2] = 0x14; // timeout 50ms * 20 = 1 second
    pn532_packetbuffer[3] = 0x01; // use IRQ pin!

    if (pn532_write_command(pn532_packetbuffer, 4))
        return false;
    return (0 < pn532_read_response(pn532_packetbuffer, sizeof(pn532_packetbuffer)));
}

bool
pn532_read_passive_target_ID(uint8_t cardbaudrate, uint8_t *uid, uint8_t *uid_length, uint16_t timeout, bool inlist) {
    pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
    pn532_packetbuffer[1] = 1;  // max 1 cards at once (we can set this to 2 later)
    pn532_packetbuffer[2] = cardbaudrate;

    if (pn532_write_command(pn532_packetbuffer, 3)) {
        return 0x0;  // command failed
    }

    // read data packet
    if (pn532_read_response(pn532_packetbuffer, sizeof(pn532_packetbuffer), timeout) < 0) {
        return 0x0;
    }

    // check some basic stuff
    /* ISO14443A card response should be in the following format:

      byte            Description
      -------------   ------------------------------------------
      b0              Tags Found
      b1              Tag Number (only one used in this example)
      b2..3           SENS_RES
      b4              SEL_RES
      b5              NFCID Length
      b6..NFCIDLen    NFCID
    */

    if (pn532_packetbuffer[0] != 1)
        return 0;

    uint16_t sens_res = pn532_packetbuffer[2];
    sens_res <<= 8;
    sens_res |= pn532_packetbuffer[3];

    /* Card appears to be Mifare Classic */
    *uid_length = pn532_packetbuffer[5];

    for (uint8_t i = 0; i < pn532_packetbuffer[5]; i++) {
        uid[i] = pn532_packetbuffer[6 + i];
    }

    if (inlist) {
        inListedTag = pn532_packetbuffer[1];
    }

    return 1;
}


uint8_t pn532_mifareclassic_authenticate_block(uint8_t *uid, uint8_t uidLen, uint32_t blockNumber, uint8_t keyNumber,
                                               uint8_t *keyData) {
    uint8_t i;

    // Hang on to the key and uid data
    memcpy(pn532_key, keyData, 6);
    memcpy(pn532_uid, uid, uidLen);
    pn532_uidLen = uidLen;

    // Prepare the authentication command //
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;   /* Data Exchange Header */
    pn532_packetbuffer[1] = 1;                              /* Max card numbers */
    pn532_packetbuffer[2] = (keyNumber) ? MIFARE_CMD_AUTH_B : MIFARE_CMD_AUTH_A;
    pn532_packetbuffer[3] = blockNumber;                    /* Block Number (1K = 0..63, 4K = 0..255 */
    memcpy(pn532_packetbuffer + 4, pn532_key, 6);
    for (i = 0; i < pn532_uidLen; i++) {
        pn532_packetbuffer[10 + i] = pn532_uid[i];              /* 4 bytes card ID */
    }

    if (pn532_write_command(pn532_packetbuffer, 10 + pn532_uidLen))
        return 0;

    // Read the response packet
    pn532_read_response(pn532_packetbuffer, sizeof(pn532_packetbuffer));

    // Check if the response is valid and we are authenticated???
    // for an auth success it should be bytes 5-7: 0xD5 0x41 0x00
    // Mifare auth error is technically byte 7: 0x14 but anything other and 0x00 is not good
    if (pn532_packetbuffer[0] != 0x00) {
        // Authentification failed
        return 0;
    }

    return 1;
}

uint8_t pn532_mifareclassic_read_data_block(uint8_t blockNumber, uint8_t *data) {
    /* Prepare the command */
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                      /* Card number */
    pn532_packetbuffer[2] = MIFARE_CMD_READ;        /* Mifare Read command = 0x30 */
    pn532_packetbuffer[3] = blockNumber;            /* Block Number (0..63 for 1K, 0..255 for 4K) */

    /* Send the command */
    if (pn532_write_command(pn532_packetbuffer, 4)) {
        return 0;
    }

    /* Read the response packet */
    pn532_read_response(pn532_packetbuffer, sizeof(pn532_packetbuffer));

    /* If byte 8 isn't 0x00 we probably have an error */
    if (pn532_packetbuffer[0] != 0x00) {
        return 0;
    }

    /* Copy the 16 data bytes to the output buffer        */
    /* Block content starts at byte 9 of a valid response */
    memcpy(data, pn532_packetbuffer + 1, 16);

    return 1;
}

int8_t
pn532_felica_polling(uint16_t systemCode, uint8_t requestCode, uint8_t *idm, uint8_t *pmm, uint16_t *systemCodeResponse,
                     uint16_t timeout) {
    pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
    pn532_packetbuffer[1] = 1;
    pn532_packetbuffer[2] = 1;
    pn532_packetbuffer[3] = FELICA_CMD_POLLING;
    pn532_packetbuffer[4] = (systemCode >> 8) & 0xFF;
    pn532_packetbuffer[5] = systemCode & 0xFF;
    pn532_packetbuffer[6] = requestCode;
    pn532_packetbuffer[7] = 0;

    if (pn532_write_command(pn532_packetbuffer, 8)) {
        // Could not send Polling command
        return -1;
    }

    int16_t status = pn532_read_response_full(pn532_packetbuffer, 22, timeout);
    if (status < 0) {
        // Could not receive response
        return -2;
    }

    // Check NbTg (pn532_packetbuffer[7])
    if (pn532_packetbuffer[0] == 0) {
        // No card had detected
        return 0;
    } else if (pn532_packetbuffer[0] != 1) {
        // Unhandled number of targets inlisted
        return -3;
    }

    inListedTag = pn532_packetbuffer[1];

    // length check
    uint8_t responseLength = pn532_packetbuffer[2];
    if (responseLength != 18 && responseLength != 20) {
        // Wrong response length
        return -4;
    }

    uint8_t i;
    for (i = 0; i < 8; ++i) {
        idm[i] = pn532_packetbuffer[4 + i];
        pn532_felicaIDm[i] = pn532_packetbuffer[4 + i];
        pmm[i] = pn532_packetbuffer[12 + i];
        pn532_felicaPMm[i] = pn532_packetbuffer[12 + i];
    }

    if (responseLength == 20) {
        *systemCodeResponse = (uint16_t) ((pn532_packetbuffer[20] << 8) + pn532_packetbuffer[21]);
    }

    return 1;
}



