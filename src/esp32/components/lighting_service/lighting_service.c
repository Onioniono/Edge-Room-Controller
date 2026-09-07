#include "lighting_service.h"
#include "ws2812.h"

#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static const char *TAG = "LIGHTING_SERVICE";    // Tag for logging

// Contains public variables of current LED state
typedef struct {
    lighting_mode_t base_mode;
    lighting_mode_t override_mode;

    bool override_active;
    
    uint8_t brightness_percent;

    uint8_t custom_red;     // Customizable input values for colors
    uint8_t custom_green;
    uint8_t custom_blue;
} lighting_state_t;

static lighting_state_t lighting_state = {
    .base_mode = LIGHTING_MODE_IDLE,
    .override_mode = LIGHTING_MODE_LISTENING,
    .override_active = false,

    .brightness_percent = 50,

    .custom_red = 0,
    .custom_green = 0,
    .custom_blue = 0
};

// Contains public variables for requested RTOS queue-based commands
typedef enum {
    LIGHTING_CMD_SET_MODE,
    LIGHTING_CMD_SET_COLOR,
    LIGHTING_CMD_SET_BRIGHTNESS,
    LIGHTING_CMD_OVERRIDE_BEGIN,
    LIGHTING_CMD_OVERRIDE_END,
    LIGHTING_CMD_OFF
} lighting_command_type_t;
typedef struct {
    lighting_command_type_t type;
    lighting_mode_t mode;
    
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    uint8_t brightness_percent;
} lighting_command_t;

static QueueHandle_t lighting_queue = NULL;     // Private Queue Handle

// ############################################################################################## //
// PRIVATE FUNCTIONS
// ############################################################################################## //

// ----------------------------------------
// Brightness Scaling function to adjust visible LED intensity
/*
Description:
This function scales the current values to the desired brightness.
Parameters:
- value: original 0-255 value of the LED color
- brightness_percent: a 0-100 value that scales the value of the color to adjust brightness
Returns:
- Scaled value of selected color
*/
// ----------------------------------------
static uint8_t lighting_scale_channel(uint8_t value, uint8_t brightness_percent) {
    // 
    if (brightness_percent > 100) {
        brightness_percent = 100;
    }
    return (uint8_t)(((uint16_t)value * brightness_percent) / 100);
}

// ----------------------------------------
// Apply different mode colors to LEDs
/*
Description:
This function sets the RGB values for a specified mode into memory and sends it to the LEDs
Parameters:
- mode: An existing mode with the enumerated list of modes (located within lighting_service
Returns:
- Uses WS2812 function to change state of LEDs relative to the different style modes
Notes:
- Only uses static colors; animations, transitions, etc. are not established here (feature TBD)
*/
// ----------------------------------------
static esp_err_t lighting_apply_mode(lighting_mode_t mode) {
    // Placeholder values
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    // Switch between different color values for different styles
    switch (mode) {
        case LIGHTING_MODE_OFF:
            // LEDs off
            red = 0;
            green = 0;
            blue = 0;
            break;
        case LIGHTING_MODE_IDLE:
            // Purple
            red = 80;
            green = 0;
            blue = 160;
            break;
        case LIGHTING_MODE_LISTENING:
            // Cyan
            red = 0;
            green = 180;
            blue = 255;
            break;
        case LIGHTING_MODE_PROCESSING:
            // Yellow
            red = 255;
            green = 210;
            blue = 60;
            break;
        case LIGHTING_MODE_MUTED:
            // Orange
            red = 255;
            green = 40;
            blue = 0;
            break;
        case LIGHTING_MODE_ERROR:
            // Red
            red = 255;
            green = 0;
            blue = 0;
            break;
        case LIGHTING_MODE_CUSTOM:
            red = lighting_state.custom_red;
            green = lighting_state.custom_green;
            blue = lighting_state.custom_blue;
            break;
        // Unknown Mode Input
        default:
            return ESP_ERR_INVALID_ARG;
    }
    // Scale Brightness
    red = lighting_scale_channel(red, lighting_state.brightness_percent);
    green = lighting_scale_channel(green, lighting_state.brightness_percent);
    blue = lighting_scale_channel(blue, lighting_state.brightness_percent);
    // Clear current LEDs in preparation for new mode
    ESP_ERROR_CHECK(ws2812_clear());
    // Transmit new mode to memory
    ESP_ERROR_CHECK(ws2812_set_all(red, green, blue));
    // Apply new mode
    return ws2812_show();
}

// -------------------------------------------
// Change state of mode to whatever is currently in memory
/*
Description:
This function changes the state of the LEDs to apply the current state of override or normal procedure
Parameters:
- None
Returns:
- Changes mode to override or base
*/
// -------------------------------------------
static esp_err_t lighting_apply_state(void) {
    if (lighting_state.override_active) {
        return lighting_apply_mode(lighting_state.override_mode);
    }
    return lighting_apply_mode(lighting_state.base_mode);
}

// ############################################################################################## //
// PUBLIC FUNCTIONS
// ############################################################################################## //

// ------------------------------------
// Change mode to a preset mode
// ------------------------------------
esp_err_t lighting_set_mode(lighting_mode_t mode) {
    // Verify requested mode exists
    if (mode < LIGHTING_MODE_OFF || mode > LIGHTING_MODE_CUSTOM) {
        return ESP_ERR_INVALID_ARG;
    }
    // Set Mode
    lighting_state.base_mode = mode;
    return lighting_apply_state();
}

// ------------------------------------
// Change LED colors to a custom static color
// ------------------------------------
esp_err_t lighting_set_color(uint8_t red, uint8_t green, uint8_t blue) {
    // Change color variables to custom inputs
    lighting_state.custom_red = red;
    lighting_state.custom_green = green;
    lighting_state.custom_blue = blue;
    // Change Mode to Custom
    lighting_state.base_mode = LIGHTING_MODE_CUSTOM;
    // Apply new state and colors
    return lighting_apply_state();
}

// ------------------------------------
// Change LED brightness
// ------------------------------------
esp_err_t lighting_set_brightness(uint8_t brightness_percent) {
    // Verify intended brightness is within range
    if (brightness_percent > 100) {
        return ESP_ERR_INVALID_ARG;
    }
    // Apply brightness
    lighting_state.brightness_percent = brightness_percent;
    return lighting_apply_state();
}

// ------------------------------------
// Start Override of lighting
// ------------------------------------
esp_err_t lighting_override_begin(lighting_mode_t mode) {
    // Verify mode exists
    if (mode < LIGHTING_MODE_OFF || mode > LIGHTING_MODE_CUSTOM) {
        return ESP_ERR_INVALID_ARG;
    }
    // Change to Override Mode
    lighting_state.override_mode = mode;
    lighting_state.override_active = true;
    return lighting_apply_state();
}

// ------------------------------------
// End Override of lighting
// ------------------------------------
esp_err_t lighting_override_end(void) {
    lighting_state.override_active = false;
    return lighting_apply_state();
}

// ------------------------------------
// Turn off LED lighting
// ------------------------------------
esp_err_t lighting_off(void) {
    return lighting_set_mode(LIGHTING_MODE_OFF);
}

// ############################################################################################## //
// RTOS FUNCTIONS
// ############################################################################################## //

// ------------------------------------
// Create lighting_service RTOS task to wait for commands
// ------------------------------------
static void lighting_task(void *arg) {
    lighting_command_t command;
    // Transfer parameters from command request to related lighting variables 
    while (1) {
        // Wait indefientely until a lighting commend is sent
        if (xQueueReceive(lighting_queue, &command, portMAX_DELAY) == pdTRUE) {
            switch (command.type) {
                case LIGHTING_CMD_SET_MODE:
                    lighting_state.base_mode = command.mode;
                    break;

                case LIGHTING_CMD_SET_COLOR:
                    lighting_state.custom_red = command.red;
                    lighting_state.custom_green = command.green;
                    lighting_state.custom_blue = command.blue;
                    lighting_state.base_mode = LIGHTING_MODE_CUSTOM;
                    break;

                case LIGHTING_CMD_SET_BRIGHTNESS:
                    lighting_state.brightness_percent = command.brightness_percent;
                    break;

                case LIGHTING_CMD_OVERRIDE_BEGIN:
                    lighting_state.override_mode = command.mode;
                    lighting_state.override_active = true;
                    break;

                case LIGHTING_CMD_OVERRIDE_END:
                    lighting_state.override_active = false;
                    break;
                
                case LIGHTING_CMD_OFF:
                    lighting_state.base_mode = LIGHTING_MODE_OFF;
                    break;
                
                default:
                    ESP_LOGW(TAG, "Unkown ligthing command");
                    continue;    
            }
            // Apply new state
            esp_err_t err = lighting_apply_state();
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to apply lighting state: %s", esp_err_to_name(err));
            }
        }
    }
}


// ------------------------------------
// Start the lighting_service RTOS task
// ------------------------------------
esp_err_t lighting_service_start(void) {
    // Create a queue to hold up to 8 lighting commands
    lighting_queue = xQueueCreate(8, sizeof(lighting_command_t));
    if (lighting_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create lighting queue");
        return ESP_ERR_NO_MEM;
    }
    // Create and start the lighting task
    BaseType_t result = xTaskCreate(lighting_task, "lighting_task", 4096, NULL, 5, NULL);
    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create lighting task");
        return ESP_FAIL;
    }
    // Logging
    ESP_LOGI(TAG, "Lighting service started");
    return ESP_OK;
}