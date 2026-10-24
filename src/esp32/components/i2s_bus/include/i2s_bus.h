#pragma once

#include "esp_err.h"
#include "esp_log.h"

esp_err_t i2s_bus_init(void);       // Initialize the I2S bus with the specified configuration and store the handle in i2s_bus_handle
esp_err_t i2s_bus_deinit(void);     // Deinitialize the I2S bus and release any resources associated with it
esp_err_t i2s_bus_read(void *buffer, size_t requested_bytes, size_t *bytes_read, uint32_t timeout_ms); // Read audio data from the I2S bus into the provided destination buffer