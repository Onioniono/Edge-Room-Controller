#include "environment_service.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "bme280.h"

#define ENVIRONMENT_TASK_PERIOD_MS 30000 // 30 seconds
#define ENVIRONMENT_TASK_STACK_SIZE 4096
#define ENVIRONMENT_TASK_PRIORITY 5

static const char *TAG = "ENVIRONMENT_SERVICE";  // Tag for logging

// ------------------------------
// Environment Task Function
/*
Description:
This function is the main task for the environment service. It runs in an infinite loop, periodically reading data from 
the BME280 sensor and logging the temperature, pressure, and humidity values.
Parameters:
- arg: A pointer to any arguments passed to the task (not used in this implementation).
Returns:
- None
Notes:
- The task uses vTaskDelay to wait for a specified period (ENVIRONMENT_TASK_PERIOD_MS) between readings.
- The bme280_read function is called to read the sensor data, and the results are logged using ESP_LOGI.
- If the sensor read fails, an error message is logged using ESP_LOGE.
*/
// ------------------------------
static void environment_task(void *arg) {
    bme280_data_t data; // Structure to hold sensor data
    while(1) {
        esp_err_t err = bme280_read(&data);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Temperature: %.2f °F, Pressure: %.2f hPa, Humidity: %.2f %%", 
                     data.temperature_f, data.pressure_hpa, data.humidity_percent);
        } else {
            ESP_LOGE(TAG, "Failed to read BME280 sensor data: %s", esp_err_to_name(err));
        }
        vTaskDelay(pdMS_TO_TICKS(ENVIRONMENT_TASK_PERIOD_MS));
    }    
}

// ------------------------------
// Environment Service Start Function
/*
Description:
This function starts the environment service by creating a FreeRTOS task that periodically 
reads data from the BME280 sensor and logs the temperature, pressure, and humidity values.
Parameters:
- None
Returns:
- ESP_OK on success, or an appropriate error code on failure.
Notes:
- The function uses xTaskCreate to create the environment task.
- The task runs indefinitely, reading sensor data every ENVIRONMENT_TASK_PERIOD_MS milliseconds.
*/
// ------------------------------
esp_err_t environment_service_start(void) {
    BaseType_t result = xTaskCreate(environment_task, "environment_task", ENVIRONMENT_TASK_STACK_SIZE, NULL, ENVIRONMENT_TASK_PRIORITY, NULL);
    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create environment task");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Environment service initialized successfully");
    return ESP_OK;
}