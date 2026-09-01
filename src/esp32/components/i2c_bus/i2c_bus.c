#include "driver/i2c_master.h"
#include <stdint.h>
#include "esp_log.h"
#include "i2c_bus.h"
#include "hardware_config.h"

static const char *TAG = "i2c_bus";   // Tag for logging
static i2c_master_bus_handle_t i2c_bus_handle = NULL;   // Handle for the I2C master bus

// ----------------------
// I2C Bus Scan Function
/*
Description:
This function scans the I2C bus for connected devices by probing each possible address in a range.
Parameters:
- found_address: A pointer to a uint8_t variable where the address of the first detected device will be stored.
Returns:
- ESP_OK if a device is found, or an appropriate error code if no devices are found or if an error occurs during probing.
Notes:
- The function iterates through the I2C address range from 0x08 to 0x77, probing each address using the i2c_master_probe function.
- If a device is found at an address, the function logs the address and stores it in the provided pointer.
- If no devices are found, the function returns ESP_ERR_NOT_FOUND.
*/
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
/*
Description:
This function initializes the I2C master bus with a specified configuration, including clock source, GPIO pins for SDA and SCL, glitch filtering, and internal pull-up settings.
Parameters:
- None
Returns:
- Returns and stores the handle for the I2C master bus in the static variable i2c_bus_handle.
Notes:
- The function creates a new I2C master bus using the i2c_new_master_bus function and stores the handle in the static variable i2c_bus_handle.
- The function also includes commented-out code for scanning the I2C bus for connected devices, which can be enabled if needed.
*/
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
/*
Description:
This function returns the handle for the I2C master bus.
Parameters:
- None
Returns:
- The handle for the I2C master bus.
Notes:
- Used publicly for other components to access the I2C bus for communication with devices.
*/
// --------------------------------
i2c_master_bus_handle_t i2c_bus_get_handle(void)
{
    return i2c_bus_handle;  // Return the handle for the I2C master bus
}