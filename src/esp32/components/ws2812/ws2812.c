#include "ws2812.h"
#include "hardware_config.h"

#include <stdint.h>
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "esp_rom_sys.h"

#define WS2812_RMT_RESOLUTION_HZ 10000000   // 10 MHz resolution for RMT
#define WS2812_RMT_MEM_SYMOLS 64            // Number of symbols in RMT memory for WS2812
#define WS2812_RMT_QUEUE_DEPTH 4            // Depth of the RMT queue for WS2812

#define WS2812_LED_COUNT 60
#define WS2812_BYTES_PER_LED 3
static uint8_t pixel_buffer[WS2812_LED_COUNT * WS2812_BYTES_PER_LED];   // Private buffer where each LED uses 3 bytes
                                                                        // Array in RAM represents desired state of entire strip

#define WS2812_RESET_TIME_US 300
#define WS2812_TX_TIMOUT_MS 100

static const char *TAG = "WS2812";  // Tag for logging

static rmt_channel_handle_t ws2812_channel = NULL;  // Handle for the RMT channel used to control the WS2812 LEDs
static rmt_encoder_handle_t ws2812_encoder = NULL;  // Handle for the RMT encoder used to encode the LED data

// -----------------------------------------
// Initialization Configurations for the ws2812 using the built-in RMT peripheral
/*
Description:
Configure and enable the RMT peripheral/encoder
Parameters:
- None
Returns:
- ESP_OK on success, or an appropriate error code on failure.
Notes:
- RMT is an esp-idf defined structure.
- At 10MHz one RMT tick is 0.1 us.
- 10MHz is used for the required finer timing.
*/
// -----------------------------------------
esp_err_t ws2812_init(void) {
    // Prevent re-initialization if the WS2812 has already been initialized
    if (ws2812_channel != NULL || ws2812_encoder != NULL) {
        ESP_LOGW(TAG, "WS2812 already initialized");
        return ESP_ERR_INVALID_STATE;
    }
    // Configure ESP32-S3 RMT transmit channel for WS2812
    rmt_tx_channel_config_t tx_channel_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .gpio_num = LED_DATA_GPIO,
        .mem_block_symbols = WS2812_RMT_MEM_SYMOLS,     // Allocates 64 waveform symbols in the RMT hardware memory
        .resolution_hz = WS2812_RMT_RESOLUTION_HZ,
        .trans_queue_depth = WS2812_RMT_QUEUE_DEPTH,    // How many RMT transmission requests can be queued
        .flags.invert_out = false,
        .flags.with_dma = false,
    };
    // Create the RMT transmit channel
    esp_err_t err = rmt_new_tx_channel(&tx_channel_config, &ws2812_channel);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create RMT TX channel: %s", esp_err_to_name(err));
        return err;
    }
    // Configure the RMT encoder for WS2812
    rmt_bytes_encoder_config_t encoder_config = {
        .bit0 = {
            // HIGH for 3 ticks, LOW for 9 ticks
            .level0 = 1,
            .duration0 = 3,
            .level1 = 0,
            .duration1 = 9,
        },
        .bit1 = {
            // HIGH for 9 ticks, LOW for 3 ticks
            .level0 = 1,
            .duration0 = 9,
            .level1 = 0,
            .duration1 = 3,
        },
        .flags.msb_first = true,
    };
    err = rmt_new_bytes_encoder(&encoder_config, &ws2812_encoder);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create WS2812 RMT encoder: %s", esp_err_to_name(err));
        return err;
    }
    // Enable the RMT channel so it can transmit data
    err = rmt_enable(ws2812_channel);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable RMT TX channel: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "WS2812 RMT initialized on GPIO%d", LED_DATA_GPIO);
    return ESP_OK;
}

// -----------------------------------------
// Set Pixel Function
/*
Description:
Modify the desired color of one LED in hte RAM buffer
Parameters:
- RGB
Returns:
- ESP_OK on success, or an appropriate error code on failure.
Notes:
- Only changes ESP32 RAM
- Does not call RMT
- Stores the data of desired LED state but does not execute the physical state
*/
// -----------------------------------------
esp_err_t ws2812_set_pixel(uint16_t index, uint8_t red, uint8_t green, uint8_t blue) {
    // Verify that requested LED exists on the strip
    if (index >= WS2812_LED_COUNT) {
        ESP_LOGE(TAG, "Invalid LED index");
        return ESP_ERR_INVALID_ARG;
    }
    // Each pixel occupies three consecutive bytes in the buffer
    size_t offset = index * WS2812_BYTES_PER_LED;
    // WS2812 expects color data in GRB order
    pixel_buffer[offset + 0] = green;
    pixel_buffer[offset + 1] = red;
    pixel_buffer[offset + 2] = blue;
    return ESP_OK;
}

// -----------------------------------------
// Show the LED Colors
/*
Description:
Transmit the current buffer to the physical LED strip
Parameters:
- None
Returns:
- ESP_OK on success, or an appropriate error code on failure.
Notes:
- Configures how a single individual transmission operates not the entire tx channel
- Takes and sends the configured pixel_buffer[] created and stored by ws2812_set_pixel() 
*/
// -----------------------------------------
esp_err_t ws2812_show(void) {
    // Verify WS2812 driver is initialized
    if (ws2812_channel == NULL || ws2812_encoder == NULL) {
        ESP_LOGE(TAG, "WS2812 driver not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    // Configure RMT transmission
    rmt_transmit_config_t tx_config = {
        .loop_count = 0,        // Transmit pixel data only once
        .flags.eot_level = 0,   // Tells RMT otuput to end in LOW state
    };
    // Transmit entire GRB pixel buffer using WS2812 RMT encoder
    esp_err_t err = rmt_transmit(ws2812_channel, ws2812_encoder, pixel_buffer, sizeof(pixel_buffer), &tx_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start WS2812 transmission: %s", esp_err_to_name(err));
        return err;
    }
    // Wait until RMT peripheral has transmitted the frame
    err = rmt_tx_wait_all_done(ws2812_channel, WS2812_TX_TIMOUT_MS);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "WS2812 transmission did not complete: %s", esp_err_to_name(err));
        return err;
    }
    // WS2812 requries data line to remain LOW long enough after frame for LEDs to latch newly received color data
    esp_rom_delay_us(WS2812_RESET_TIME_US);

    return ESP_OK;
}

// -----------------------------------------
// Clear the LED Colors
/*
Description:
Set every LED in the buffer to 0,0,0
Parameters:
- None
Returns:
- ESP_OK on success, or an appropriate error code on failure.
Notes:
- Fills the entire pixel_buffer[] with zero-valued bytes.
- Only changes the memory; it does not execute the physical LED state.
*/
// -----------------------------------------
esp_err_t ws2812_clear(void) {
    memset(pixel_buffer,0,sizeof(pixel_buffer));
    return ESP_OK;
}

// -----------------------------------------
// Apply values to all the LEDs
/*
Description:
Set every LED in the buffer to defined RGB values
Parameters:
- red, green, blue: 0-255 values to define color
Returns:
- ESP_OK on success, or an appropriate error code on failure.
*/
// -----------------------------------------
esp_err_t ws2812_set_all(uint8_t red,uint8_t green,uint8_t blue) {
    for (uint16_t i = 0; i < WS2812_LED_COUNT; i++) {
        esp_err_t err = ws2812_set_pixel(i, red, green, blue);
        if (err != ESP_OK) {
            return err;
        }
    }
    return ESP_OK;
}