#include "system_manager.h"
#include "i2c_bus.h"
#include "system_config.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "bme280.h"
#include "environment_service.h"
#include "ws2812.h"
#include "lighting_service.h"

static const char *TAG = "SYSTEM_MANAGER";  // Tag for logging

// ------------------------------
// System Manager Initialization Function
/*
Description:
This function initializes the system manager.
- Includes: 
    - I2C bus initialization
    - BME280 sensor initialization
    - Environment service initialization (RTOS task; 30 second periodic BME280 sensor readings)
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
    
    // Initialize the I2C bus
    #if ENABLE_I2C_BUS
        ESP_ERROR_CHECK(i2c_bus_init());
    #endif
    
    // Initialize the BME280 sensor if enabled
    #if ENABLE_BME280
        ESP_ERROR_CHECK(bme280_init());
    #endif
    
    // Initialize Light WS2812B if enabled
    #if ENABLE_WS2812E
        ESP_ERROR_CHECK(ws2812_init());
    #endif

    // Return ESP_OK to indicate successful initialization
    return ESP_OK;
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

    // Start the environment service if BME280 is enabled
    #if ENABLE_BME280
        ESP_ERROR_CHECK(environment_service_start());
    #endif

    // Return ESP_OK to indicate successful start
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
*/
// ------------------------------
esp_err_t system_manager_test(void) {
    // BME280 sensor test
    #if TEST_BME280
        bme280_data_t sensor_data;
        ESP_ERROR_CHECK(bme280_read(&sensor_data));
        ESP_LOGI(TAG, "BME280 Readings - Temperature: %.2f C, Pressure: %.2f hPa, Humidity: %.2f %%", 
                sensor_data.temperature_c, sensor_data.pressure_hpa, sensor_data.humidity_percent);
    #endif

    // WS2812E LED test
    #if TEST_WS2812
        ESP_ERROR_CHECK(ws2812_set_pixel(0,255,0,0));   // First LED Red
        ESP_ERROR_CHECK(ws2812_show());
        vTaskDelay(pdMS_TO_TICKS(1000));    // delay for observation

        ESP_ERROR_CHECK(ws2812_set_pixel(0,0,255,0));   // First LED Green
        ESP_ERROR_CHECK(ws2812_show());
        vTaskDelay(pdMS_TO_TICKS(1000));    // delay

        ESP_ERROR_CHECK(ws2812_set_pixel(0,0,0,255));   // First LED Blue
        ESP_ERROR_CHECK(ws2812_show());
        vTaskDelay(pdMS_TO_TICKS(1000));    // delay

        ESP_ERROR_CHECK(ws2812_clear());    // All LED off
        ESP_ERROR_CHECK(ws2812_show());

        ESP_ERROR_CHECK(ws2812_set_pixel(0,128,0,255));     // Purple
        ESP_ERROR_CHECK(ws2812_set_pixel(1,255,80,0));      // Orange
        ESP_ERROR_CHECK(ws2812_set_pixel(2,0,255,255));     // Cyan
        ESP_ERROR_CHECK(ws2812_show());
        vTaskDelay(pdMS_TO_TICKS(10000));

        ESP_ERROR_CHECK(ws2812_clear());    // All LED off
        ESP_ERROR_CHECK(ws2812_show());
    #endif

    // lighting_service test
    #if TEST_LIGHTING_SERVICE
            lighting_set_color(128, 0, 255);    // purple base
            vTaskDelay(pdMS_TO_TICKS(5000));
            lighting_set_brightness(10);        // dim LED
            vTaskDelay(pdMS_TO_TICKS(5000));
            lighting_override_begin(LIGHTING_MODE_LISTENING);
            vTaskDelay(pdMS_TO_TICKS(5000));
            lighting_override_end();
            vTaskDelay(pdMS_TO_TICKS(5000));
            lighting_off();
    #endif

   return ESP_OK; // Placeholder return value for testing purposes
}