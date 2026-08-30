#pragma once

#include "esp_err.h"
#include "driver/i2c_master.h"

typedef struct {
    float temperature_c;  // Temperature in degrees Celsius
    float pressure_hpa;   // Pressure in hectopascals
    float humidity_percent; // Humidity in percentage
} bme280_data_t;

esp_err_t bme280_init(void);
esp_err_t bme280_read(bme280_data_t *data);