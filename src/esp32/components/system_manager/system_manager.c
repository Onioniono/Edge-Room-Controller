#include "system_manager.h"
#include "i2c_bus.h"
#include "bme280.h"

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