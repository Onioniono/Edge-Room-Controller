#pragma once

#include "esp_err.h"
#include "driver/i2c_master.h"

// ------------------------------
// BME280 Sensor Data Structure
/*
Description:
This structure holds the data read from the BME280 environmental sensor, including temperature, pressure, and humidity readings.
Notes:
- The temperature is represented in degrees Celsius, pressure in hectopascals (hPa), and humidity as a percentage.
- The structure is used to pass sensor data between functions and components in the system.
*/
// ------------------------------
typedef struct {
    float temperature_f;  // Temperature in degrees Fahrenheit
    float pressure_hpa;   // Pressure in hectopascals
    float humidity_percent; // Humidity in percentage
} bme280_data_t;

esp_err_t bme280_init(void);                    // Initialize the BME280 sensor and configure it for measurement
esp_err_t bme280_read(bme280_data_t *data);     // Read temperature, pressure, and humidity data from the BME280 sensor and store it in the provided structure