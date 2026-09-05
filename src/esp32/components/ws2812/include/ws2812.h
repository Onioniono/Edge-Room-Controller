#pragma once

#include "esp_err.h"
#include <stdint.h>

esp_err_t ws2812_init(void);
esp_err_t ws2812_set_pixel(uint16_t index, uint8_t red, uint8_t green, uint8_t blue);
esp_err_t ws2812_show(void);
esp_err_t ws2812_clear(void);