#include "system_manager.h"
#include "i2c_bus.h"
#include "bme280.h"
#include "esp_log.h"

static const char *TAG = "SYSTEM_MANAGER";  // Tag for logging

// ------------------------------
// System Manager Initialization Function
/*
Description:
This function initializes the system manager.
- Includes: 
    - I2C bus initialization
    - BME280 sensor initialization
    - Other system services (commented out for now)
Parameters:
- None
Returns:
- ESP_OK on success, or an appropriate error code on failure.
Notes:
- The function uses ESP_ERROR_CHECK to ensure that each initialization step is successful.
- If any initialization step fails, the function will log the error and return the corresponding error code
*/
// ------------------------------
esp_err_t system_manager_init(void)
{
    // Initialize system manager resources here
    ESP_LOGI(TAG, "Initializing system manager...");
    ESP_ERROR_CHECK(i2c_bus_init());
    ESP_ERROR_CHECK(bme280_init());
    /*
    input_service_init();
    sensor_service_init();
    lighting_service_init();
    audio_service_init();
    voice_ai_init();
    network_service_init();
    pi_link_init();
    */
    return ESP_OK;
}

// ------------------------------
// System Manager One-Shot Test Function
/*
Description:
This function performs a one-shot test of the system manager.
Parameters:
- None
Returns:
- ESP_OK on success, or an appropriate error code on failure.
Notes:
- Can be commented out if not needed.
- Only used for testing purposes to verify that the system manager and its components are functioning correctly.
- Expected outputs through esp logs in the terminal.
*/
// ------------------------------
esp_err_t system_manager_test(void) {
    // BME280 sensor test
    // /*
    bme280_data_t sensor_data;
    ESP_ERROR_CHECK(bme280_read(&sensor_data));
    ESP_LOGI(TAG, "BME280 Readings - Temperature: %.2f C, Pressure: %.2f hPa, Humidity: %.2f %%", 
             sensor_data.temperature_c, sensor_data.pressure_hpa, sensor_data.humidity_percent);

    return ESP_OK;
    // */
}

// ------------------------------
// System Manager Start Function
/*
Description:
This function starts the system manager operations.
Parameters:
- None
Returns:
- ESP_OK on success, or an appropriate error code on failure.
Notes:
- The function uses ESP_ERROR_CHECK to ensure that each start step is successful.
- If any start step fails, the function will log the error and return the corresponding error code.
*/
// ------------------------------
esp_err_t system_manager_start(void)
{
    // Start system manager operations here
    ESP_LOGI(TAG, "Starting system manager...");
    /*
    input_service_start();
    sensor_service_start();
    lighting_service_start();
    audio_service_start();
    voice_ai_start();
    network_service_start();
    pi_link_start();
    */
    return ESP_OK;
}