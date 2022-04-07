#include "driver/rmt.h"
#include "led_strip.h"
#include "ws2812.h"

/**
 * @brief Simple helper function, converting HSV color space to RGB color space
 *
 * Wiki: https://en.wikipedia.org/wiki/HSL_and_HSV
 *
 */
void led_strip_hsv2rgb(uint32_t h, uint32_t s, uint32_t v, uint32_t *r, uint32_t *g, uint32_t *b) {
    h %= 360; // h -> [0,360]
    uint32_t rgb_max = v * 2.55f;
    uint32_t rgb_min = rgb_max * (100 - s) / 100.0f;

    uint32_t i = h / 60;
    uint32_t diff = h % 60;

    // RGB adjustment amount by hue
    uint32_t rgb_adj = (rgb_max - rgb_min) * diff / 60;

    switch (i) {
        case 0:
            *r = rgb_max;
            *g = rgb_min + rgb_adj;
            *b = rgb_min;
            break;
        case 1:
            *r = rgb_max - rgb_adj;
            *g = rgb_max;
            *b = rgb_min;
            break;
        case 2:
            *r = rgb_min;
            *g = rgb_max;
            *b = rgb_min + rgb_adj;
            break;
        case 3:
            *r = rgb_min;
            *g = rgb_max - rgb_adj;
            *b = rgb_max;
            break;
        case 4:
            *r = rgb_min + rgb_adj;
            *g = rgb_min;
            *b = rgb_max;
            break;
        default:
            *r = rgb_max;
            *g = rgb_min;
            *b = rgb_max - rgb_adj;
            break;
    }
}

uint16_t h;
led_strip_t *led;

// TODO Add static button LED
// TODO Add side LED

void led_init() {
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(GPIO_NUM_18, RMT_CHANNEL_0);
    config.clk_div = 2;
    rmt_config(&config);
    rmt_driver_install(config.channel, 0, 0);

    led_strip_config_t led_config = LED_STRIP_DEFAULT_CONFIG(6, (led_strip_dev_t) config.channel);
    led = led_strip_new_rmt_ws2812(&led_config);

    led->clear(led, 50);
    h = 0;

    led->set_pixel(led, 0, 0xFF, 0xFF, 0xFF);
    led->refresh(led, 100);
}


void led_update(color_t *colors) {
    for (uint32_t i = 0; i < 6; i++) {
        color_t color = colors[i];

        led->set_pixel(led, i, color.r, color.g, color.b);
    }

    led->refresh(led, 100);
}
