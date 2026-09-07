#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "esp_log.h"

// ------------------------------
// Contains different lighting mode styles for WS2812 LEDs
// ------------------------------
typedef enum {
    LIGHTING_MODE_OFF,
    LIGHTING_MODE_IDLE,
    LIGHTING_MODE_LISTENING,
    LIGHTING_MODE_PROCESSING,
    LIGHTING_MODE_MUTED,
    LIGHTING_MODE_ERROR,
    LIGHTING_MODE_CUSTOM
} lighting_mode_t;

esp_err_t lighting_service_start(void);                                     // Create FreeRTOS lighting task and the queue for event driven management
esp_err_t lighting_set_mode(lighting_mode_t mode);                          // Change the base lighting mode to a preset style
esp_err_t lighting_set_brightness(uint8_t brightness);                      // Update global brightness scale for LEDs
esp_err_t lighting_set_color(uint8_t red, uint8_t green, uint8_t blue);     // Arbitray custom RGB values
esp_err_t lighting_override_begin(lighting_mode_t mode);                    // Override current lighting style during certain processing task for visual indications
esp_err_t lighting_override_end(void);                                      // End override lighting style and return to former/default style
esp_err_t lighting_off(void);                                               // Turn off the LEDs