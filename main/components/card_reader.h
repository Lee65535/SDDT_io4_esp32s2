#pragma once

//
// cu500 nfc module define
//

#pragma pack(push, 1)
typedef struct cu500_frame_header {
    uint8_t length;
    uint8_t module_address;
    uint8_t command;
    uint8_t data[1];
} cu500_frame_header_t;
#pragma pack(pop)

inline uint8_t cu500_calc_checksum(cu500_frame_header_t *header) {
    uint32_t sum = 0;
    for (size_t i = 0; i < header->length - 1; i++) {
        sum += ((uint8_t *) header)[i];
    }

    return ~(sum & 0xFF);
}

void card_reader_init();
void card_reader_task(void *);
uint64_t get_felica_id();
