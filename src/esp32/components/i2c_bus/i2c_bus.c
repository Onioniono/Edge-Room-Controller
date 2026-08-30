#include "driver/i2c_master.h"
#include <stdint.h>
#include "esp_log.h"
#include "i2c_bus.h"
#include "hardware_config.h"

static const char *TAG = "i2c_bus";   // Tag for logging
static i2c_master_bus_handle_t i2c_bus_handle = NULL;   // Handle for the I2C master bus

// ----------------------
// I2C Bus Scan Function
// ----------------------
static esp_err_t i2c_bus_scan(uint8_t *found_address) {
    if (found_address == NULL) {
        return ESP_ERR_INVALID_ARG;  // Return error if the provided pointer is NULL
    }
    
    for (uint8_t address = 0x08; address <= 0x77; address++) {
        esp_err_t err = i2c_master_probe(i2c_bus_handle, address, 50);  // Probe the I2C bus for the device at the given address with a timeout of 50ms
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Device found at address 0x%02X", address);  // Log the detected device address
            *found_address = address;  // Store the detected address in the provided pointer
            return ESP_OK;  // Return success if a device is found
        }
        if (err != ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "I2C probe failed at address 0x%02X: %s", address, esp_err_to_name(err));  // Log any errors encountered during probing
            return err;
        }
    }
    return ESP_ERR_NOT_FOUND;  // Return error if no device is found
}

// --------------------------------
// I2C Bus Initialization Function
// --------------------------------
esp_err_t i2c_bus_init(void)
{
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,                      // Use one of the ESP32's I2C ports
        .scl_io_num = I2C_SCL_GPIO,
        .sda_io_num = I2C_SDA_GPIO,
        .glitch_ignore_cnt = 7,                     // Ignore glitches shorter than 7 clock cycles
        .flags.enable_internal_pullup = true,       // Enable internal pull-up resistors for SDA and SCL lines
    };
    return i2c_new_master_bus(&bus_config, &i2c_bus_handle);    // Create a new I2C master bus with the specified configuration and store the handle in i2c_bus_handle

    // -------------------------------
    // Scan Bus for connected devices
    // -------------------------------
    /*
    esp_err_t err = i2c_new_master_bus(&bus_config, &i2c_bus_handle);
    if (err != ESP_OK) {
        return err;
    }
    uint8_t detected_address = 0;
    err = i2c_bus_scan(&detected_address);
    if (err != ESP_OK) {
        return err;
    }
    return ESP_OK;
    */
}

// --------------------------------
// Get I2C Bus Handle Function
// --------------------------------
i2c_master_bus_handle_t i2c_bus_get_handle(void)
{
    return i2c_bus_handle;  // Return the handle for the I2C master bus
}