#pragma once

typedef struct {
    uint8_t g;
    uint8_t r;
    uint8_t b;
} color_t;

void led_init();
void led_update(color_t* color);