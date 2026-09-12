#include "input_service.h"
#include "hardware_config.h"

#include <stdbool.h>
#include <stdint.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// GPIO setup, internal switch state, debounce logic, FreeRTOS polling task, change detection

static const char *TAG = "INPUT_SERVICE";    // Tag for logging
#define INPUT_POLL_PERIOD_MS 500
#define INPUT_TASK_STACK_SIZE 3072
#define INPUT_TASK_PRIORITY 4

typedef struct {
    // GPIO assigned to "#define MIC_MUTE_GPIO GPIO_NUM_10"
    input_switch_id_t id;

    bool current_state;     // current logical active/inactive state
    bool previous_state;    // previous logical state

    bool stable_state;      // Last state officially accepted
    uint16_t debounce_counter;
} input_switch_t;

static input_switch_t mic_switch = {
    .id = INPUT_SWITCH_MIC_MUTE,
    .current_state = false,
    .previous_state = false
};
static input_switch_t cloud_switch = {
    .id = INPUT_SWITCH_CLOUD_ENABLE,
    .current_state = false,
    .previous_state = false
};

// ############################################################################################## //
// PRIVATE FUNCTIONS
// ############################################################################################## //

// ----------------------------------------
// Initialize GPIO
/*
Description:
This function initializes GPIO pins for the microphone and cloud state switches.
Parameters:
- None
Returns:
- ESP_OK if successful, or an appropriate error code if failed.
*/
// ----------------------------------------
static esp_err_t input_gpio_init(void) {
    // Create GPIO configuration structure
    gpio_config_t config = {
        .pin_bit_mask = (1ULL << MIC_MUTE_GPIO) | (1ULL << CLOUD_ENABLE_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,   // Active-low wiring
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE      // Polling approach: interrupts not needed initially
    };
    esp_err_t err = gpio_config(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Switch GPIO Config failed: %s", esp_err_to_name(err));
    }

    return ESP_OK;
}

// ------------------------------------
// Read and update input switch state
/*
Description:
This function takes the GPIO states for a given switch.
Parameters:
- gpio: Defined GPIO number
- *switch_state: which manual input switch being processed
Returns:
- None
Notes:
- Compares gpio_get_level to 0; allows active-low configuration
- Debounce logic ensures input change was stable before confirmation
*/
// ----------------------------------------
static void input_process_switch(gpio_num_t gpio, input_switch_t *switch_state) {
    // Read switch GPIO
    switch_state->current_state = (gpio_get_level(gpio) == 0);
        // Compare newest state against previous confirmed state
    if (switch_state->current_state != switch_state->previous_state) {
        // Log State Change
        ESP_LOGI(TAG, "Switch CHANGED");
        // Debounce Logic
            // Current RTOS is set to 500ms polling; debounce unnecessary currently
        // Log Type of Input Change
        switch (switch_state->id) {
            case INPUT_SWITCH_MIC_MUTE:
                if (switch_state->current_state) {
                    ESP_LOGI(TAG, "Microphone ENABLED");
                } else {
                    ESP_LOGI(TAG, "Microphone DISABLED");
                }
                break;
            case INPUT_SWITCH_CLOUD_ENABLE:
                if (switch_state->current_state) {
                    ESP_LOGI(TAG, "Cloud ENABLED");
                } else {
                    ESP_LOGI(TAG, "Cloud DISABLED");
                }
                break;
            default:
                ESP_LOGW(TAG, "Unknown Input Device Change");
                break;
        }
    }
    // Update Previous State
    switch_state->previous_state = switch_state->current_state;
}

// ------------------------------------
// Initialize input switch state
/*
Description:
This function initializess the GPIO states for a given switch.
Parameters:
- gpio: Defined GPIO number
- *switch_state: which manual input switch being processed
Returns:
- None
Notes:
- Compares gpio_get_level to 0; allows active-low configuration
*/
// ----------------------------------------
static void input_initialize_switch_state(gpio_num_t gpio, input_switch_t *switch_state) {
    // Initialize stored switch state
    switch_state->current_state = (gpio_get_level(gpio) == 0);
    switch_state->previous_state = switch_state->current_state;
}

// ############################################################################################## //
// RTOS FUNCTIONS
// ############################################################################################## //

// ------------------------------------
// Create input_service RTOS task
// ----------------------------------------
static void input_task(void *arg) {
    while (1) {
        // Read micropohone switch GPIO
        input_process_switch(MIC_MUTE_GPIO, &mic_switch);
        // Read cloud switch GPIO
        input_process_switch(CLOUD_ENABLE_GPIO, &cloud_switch);

        // Periodic Delay
        vTaskDelay(pdMS_TO_TICKS(INPUT_POLL_PERIOD_MS));
    }
}

// ------------------------------------
// Start the input_service RTOS task
// ------------------------------------
esp_err_t input_service_start(void) {
    // Verify GPIO Configuration
    esp_err_t err = input_gpio_init();
    if (err != ESP_OK) {
        return err;
    }

    // Initlialize stored switch states
    input_initialize_switch_state(MIC_MUTE_GPIO, &mic_switch);
    input_initialize_switch_state(CLOUD_ENABLE_GPIO, &cloud_switch);

    // Start task
        // result = xTaskCreate(input task, name, stack, parameters, priority, optional handle);
    BaseType_t result;
    result = xTaskCreate(input_task,"input_task",INPUT_TASK_STACK_SIZE, NULL, INPUT_TASK_PRIORITY, NULL);
    if (result != pdPASS) {
        return ESP_FAIL;
    }
    return ESP_OK;
}