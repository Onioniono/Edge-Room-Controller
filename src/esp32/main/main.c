#include <stdio.h>
#include "esp_err.h"
#include "system_manager.h"

// ------------------------------
// Main Application Entry Point
/*
Description:
This function serves as the entry point for the application. It initializes and starts the system manager,
which in turn initializes and starts various system services and components.
Parameters:
- None
Returns:
- None
Notes:
- The function uses ESP_ERROR_CHECK to ensure that each initialization and start step is successful.
- If any step fails, the function will log the error and return the corresponding error code.
*/
// ------------------------------
void app_main(void)
{
    ESP_ERROR_CHECK(system_manager_init());
    ESP_ERROR_CHECK(system_manager_start());
}