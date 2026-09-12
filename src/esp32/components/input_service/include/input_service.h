#pragma once

#include "esp_err.h"
#include "esp_log.h"

typedef enum {
    // States representing microhpone control
    MUTED,
    UNMUTED
} microphone_switch_state_t;

typedef enum {
    // States representing cloud-access control
    ENABLED,
    DISABLED
} cloud_switch_state_t;

typedef enum {
    INPUT_SWITCH_MIC_MUTE,
    INPUT_SWITCH_CLOUD_ENABLE
} input_switch_id_t;

esp_err_t input_service_start(void);

microphone_switch_state_t input_get_microphone_state(void);
cloud_switch_state_t input_get_cloud_state(void);