#include "i2s_bus.h"
#include "driver/i2s_std.h"
#include "hardware_config.h"

#define I2S_SAMPLE_RATE_HZ 16000
static i2s_chan_handle_t rx_handle = NULL;

// ############################################################################################## //
// PUBLIC FUNCTIONS
// ############################################################################################## //

// ----------------------------------------
// Initialize I2S Bus
/*
Description:
This function initializes the I2S bus with the specified configuration and stores the handle in rx_handle.
Parameters:
- None
Returns:
- ESP_OK if successful, or an appropriate error code if failed.
Notes:
- The I2S bus is configured for the INMP441 microphone, which outputs 24-bit audio data at a sample rate of 16 kHz.
- The I2S bus is configured in standard mode with a sample width of 32 bits and 2 channels (stereo).
*/
// ----------------------------------------
esp_err_t i2s_bus_init(void) {
    esp_err_t err;
    // COnfigure I2S channel (which I2S controller, DMA buffer count, DMA frame size, etc.)
        // Use default configuration for I2S channel:
            // Note: I2S_NUM_AUTO allows the driver to automatically select an available I2S peripheral
            // Note: I2S_ROLE_MASTER configures the ESP32 as the master device, generating the clock signals for the I2S bus
    i2s_chan_config_t channel_config = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    
    // Create I2S RX Channel
        // i2s_new_channel(channel_config, tx_handle, rx_handle)
    err = i2s_new_channel(&channel_config, NULL, &rx_handle);
    if (err != ESP_OK) {
        return err;
    }
        // Configure I2S Standard Mode
    i2s_std_config_t std_config = {
            // Physical ESP32 GPIO pins
                // Where the signals are physically connected
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_BCLK_GPIO,
            .ws = I2S_WS_GPIO,
            .dout = I2S_GPIO_UNUSED,
            .din = I2S_SD_GPIO
        },
            // Audio clock configuration (sample rate, clock source, etc.)
                // When the signals are transmitted
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(I2S_SAMPLE_RATE_HZ),  // Set sample rate to 16 kHz
            // Audio data/slot configuration (sample width, left/right channel, etc.)
                // How the data bits are organized
                    // Note: INMP441 outputs 24-bit audio data; WakeNet/ESP-SR expects 16-bit audio data
                        // Audio Service Directory: Convert 24-bit signed audio to 16-bit signed PCM audio data
        .slot_cfg = I2S_STD_PHILIP_SLOT_DEFAULT_CONFIG(32, 2)  // Set sample width to 32 bits and 2 channels (stereo)
    };
        // Apply Standard-Mode Configuratoin
    err = i2s_channel_init_std_mode(rx_handle, &std_config);
    if (err != ESP_OK) {
        return err;
    }
        // Enable I2S RX Channel
    err = i2s_channel_enable(rx_handle);
    if (err != ESP_OK) {
        return err;
    }

    return ESP_OK;
}

// ----------------------------------------
// Deinitialize I2S Bus
/*
Description:
This function deinitializes the I2S bus and releases any resources associated with it.
Parameters:
- None
Returns:
- ESP_OK if successful, or an appropriate error code if failed.
*/
// ----------------------------------------
esp_err_t i2s_bus_deinit(void) {
    esp_err_t err;
    // Disable I2S RX Channel
    err = i2s_channel_disable(rx_handle);
    if (err != ESP_OK) {
        return err;
    }
    // Delete I2S RX Channel
    err = i2s_del_channel(rx_handle);
    if (err != ESP_OK) {
        return err;
    }
    rx_handle = NULL;

    return ESP_OK;
}

// ----------------------------------------
// Read I2S Bus
/*
Description:
This function reads audio data from the I2S bus into the provided destination buffer.
Parameters:
- currently n/a
Returns:
- currently n/a
Notes:
- currently n/a
*/
// ----------------------------------------
esp_err_t i2s_bus_read(
    /* destination buffer */
    /* number of bytes requested */
    /* number of bytes actually received */
    /* timeout */
);