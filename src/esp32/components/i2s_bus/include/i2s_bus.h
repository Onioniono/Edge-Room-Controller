#pragma once

#include "esp_err.h"

esp_err_t i2s_bus_init(void);   // Initialize the I2S bus with the specified configuration and store the handle in i2s_bus_handle
esp_err_t i2s_deinit(void);     // Deinitialize the I2S bus and release any resources associated with it