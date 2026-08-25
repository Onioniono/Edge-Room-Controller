#include <stdio.h>
#include "esp_err.h"
#include "system_manager.h"

void app_main(void)
{
    ESP_ERROR_CHECK(system_manager_init());
    ESP_ERROR_CHECK(system_manager_start());
}