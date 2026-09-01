#pragma once

#include "esp_err.h"
#include "driver/i2c_master.h"

esp_err_t i2c_bus_init(void);                       // Initialize the I2C master bus with the specified configuration and store the handle in i2c_bus_handle
i2c_master_bus_handle_t i2c_bus_get_handle(void);   // Return the handle for the I2C master bus