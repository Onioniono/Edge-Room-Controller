#pragma once

#include "esp_err.h"
#include "esp_log.h"
#include "stdbool.h"

esp_err_t audio_service_init(void);     // Initialize auudio service resources
esp_err_t audio_service_start(void);    // Start audio service operations
esp_err_t audio_service_stop(void);     // Stop audio service operations

esp_err_t audio_service_set_mute(bool mute);    // Enable/disable microphone