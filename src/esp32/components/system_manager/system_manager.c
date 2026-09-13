#include "system_manager.h"
#include "system_config.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "i2c_bus.h"
#include "bme280.h"
#include "environment_service.h"
#include "ws2812.h"
#include "lighting_service.h"
#include "input_service.h"

static const char *TAG = "SYSTEM_MANAGER";  // Tag for logging
static QueueHandle_t system_queue = NULL;     // Private Queue Handle
#define SYSTEM_MANAGER_TASK_STACK_SIZE 3072
#define SYSTEM_MANAGER_TASK_PRIORITY 3

// Struct used for storing messaged events/commands
typedef struct {
    system_event_type_t type;

    // Event values
    bool state;
} system_event_t;

// Struct for storing most recent remembered conditions
typedef struct {
    bool microphone_muted;
    bool cloud_enabled;
} system_state_t;
static system_state_t system_state;

// ############################################################################################## //
// RTOS FUNCTIONS
// ############################################################################################## //

static void system_manager_task(void *arg) {
    system_event_t event;

    while (1) {
        // Wait until a component/subsystem reports an event
        if (xQueueReceive(system_queue, &event, portMAX_DELAY) == pdTRUE) {
            switch (event.type) {
                // Microphone Mute
                case SYSTEM_EVENT_MIC_MUTE_CHANGED:
                    system_state.microphone_muted = event.state;
                    if (event.state) {
                        // Insert muted lighting behavior
                        // Later notify audio service (when created)
                    } else {
                        // Restore normal behavior
                    }
                    system_manager_input_acknowledgement();
                    break;
                // Cloud Enabled
                case SYSTEM_EVENT_CLOUD_ENABLE_CHANGED:
                    system_state.cloud_enabled = event.state;
                    if (event.state) {
                        // Cloud access enabled
                    } else {
                        // Enter local-only behavior
                    }
                    system_manager_input_acknowledgement();
                    break;
                // default
                default:
                    ESP_LOGE(TAG, "Unknown Command Received");
                    break;
            }
        }
    }
}

// ############################################################################################## //
// PUBLIC FUNCTIONS
// ############################################################################################## //

// ------------------------------
// System Manager Initialization Function
/*
Description:
This function initializes the system manager.
- Includes: 
    - I2C bus initialization
    - BME280 sensor initialization
    - Environment service initialization (RTOS task; 30 second periodic BME280 sensor readings)
    - WS2812E LEDs initialization
    - Lighting service initialization (RTOS task; queue-based)
    - Input Service initialization (RTOS task; 500 ms periodic reading)
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
- Creates a queue system as an intermediary between different service interactions.
- The function uses ESP_ERROR_CHECK to ensure that each start step is successful.
- If any start step fails, the function will log the error and return the corresponding error code.
*/
// ------------------------------
esp_err_t system_manager_start(void)
{
    // Start system manager operations here
    ESP_LOGI(TAG, "Starting system manager...");

    // Create system manager queue
    system_queue = xQueueCreate(8, sizeof(system_event_t));
    if (system_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create system queue");
        return ESP_ERR_NO_MEM;
    }
        // Create system manager RTOS task
    BaseType_t result = xTaskCreate(system_manager_task, "system task", SYSTEM_MANAGER_TASK_STACK_SIZE, NULL, SYSTEM_MANAGER_TASK_PRIORITY, NULL);
        // Validate Task Creation
    if (result != pdPASS) {
        return ESP_FAIL;
    }

    // Start the environment service if BME280 is enabled
    #if ENABLE_BME280
        ESP_ERROR_CHECK(environment_service_start());
    #endif

    // Start lighting service if WS2812E is enabled
    #if ENABLE_WS2812E
        ESP_ERROR_CHECK(lighting_service_start());
    #endif

    // Start input service if input switches is enabled
    #if ENABLE_INPUT_SWITCHES
        ESP_ERROR_CHECK(input_service_start());
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
    // Start system manager operations here
    ESP_LOGI(TAG, "Starting system one-shot test(s)...");
    
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
            vTaskDelay(pdMS_TO_TICKS(3000));
            lighting_set_brightness(10);        // dim LED
            vTaskDelay(pdMS_TO_TICKS(3000));
            lighting_set_mode(LIGHTING_MODE_ERROR);
            vTaskDelay(pdMS_TO_TICKS(3000));
            lighting_set_mode(LIGHTING_MODE_PROCESSING);
            vTaskDelay(pdMS_TO_TICKS(3000));
            lighting_set_mode(LIGHTING_MODE_IDLE);
            vTaskDelay(pdMS_TO_TICKS(3000));
            lighting_set_mode(LIGHTING_MODE_LISTENING);
            vTaskDelay(pdMS_TO_TICKS(3000));
            lighting_override_begin(LIGHTING_MODE_IDLE);
            vTaskDelay(pdMS_TO_TICKS(3000));
            lighting_override_end();
            lighting_off();
    #endif

   return ESP_OK; // Placeholder return value for testing purposes
}

esp_err_t system_manager_send_event(system_event_type_t type, bool state) {
    // Verify Queue Exists
    if (system_queue == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    // Create event message
    system_event_t event = {
        .type = type,
        .state = state
    };

    // Send the event to the system-manager queue
    if (xQueueSend(system_queue, &event, pdMS_TO_TICKS(100)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }
    return ESP_OK;
}

// ############################################################################################## //
// PRIVATE FUNCTIONS
// ############################################################################################## //

static esp_err_t system_manager_input_acknowledgement(void) {
    // Temporarily indicate that a physical input was accepted
    esp_err_t err = lighting_temporary_override(LIGHTING_MODE_ACKNOWLEDGEMENT, 500);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to acknowledge input with lighting service");
    }
    return ESP_OK;
}