#pragma once
#include "esp_err.h"
#include <stdbool.h>

typedef enum {
    SYSTEM_EVENT_MIC_MUTE_CHANGED,
    SYSTEM_EVENT_CLOUD_ENABLE_CHANGED,
    SYSTeM_EVENT_ERROR_SET,
    SYSTEM_EVENT_ERROR_CLEARED
} system_event_type_t;

esp_err_t system_manager_init(void);
esp_err_t system_manager_test(void);
esp_err_t system_manager_start(void);

esp_err_t system_manager_send_event(system_event_type_t type, bool state);