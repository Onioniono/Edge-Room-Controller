#include "audio_service.h"

/************************* 
Purpose: 
Continuously acquire audio, manage application buffers, prepare samples, coordinate speech recognition
*************************/

// ############################################################################################## //
// PRIVATE FUNCTIONS
// ############################################################################################## //
// ----------------------------------------
// Convert 24-bit audio to 16-bit PCM
/*
Description:
Convert INMP441 data into 16-bit PCM audio data for WakeNet/ESP-SR
Parameters:
- currently n/a
Returns:
- currently n/a
Notes:
- currently n/a
*/
// ----------------------------------------
static esp_err_t audio_convert_samples();

// ----------------------------------------
// Process Audio Samples
/*
Description:
Feed prepared audio into the processing pipeline for speech recognition
Parameters:
- currently n/a
Returns:
- currently n/a
Notes:
- currently n/a
*/
// ----------------------------------------
static esp_err_t audio_process_samples();

// ############################################################################################## //
// PUBLIC FUNCTIONS
// ############################################################################################## //
// ----------------------------------------
// Initialize Audio Service
/*
Description:
Initialize the audio subsystem and necessary resources for audio acquisition and processing
Parameters:
- currently n/a
Returns:
- currently n/a
Notes:
- currently n/a
*/
// ----------------------------------------
esp_err_t audio_service_init(void) {
    // Initialize audio service resources here
    return ESP_OK;
}

// ----------------------------------------
// Stop Audio Service
/*
Description:
Stop audio processing and release resources when needed
Parameters:
- currently n/a
Returns:
- currently n/a
Notes:
- currently n/a
*/
// ----------------------------------------
esp_err_t audio_service_stop(void) {
    // Stop audio service operations here
    return ESP_OK;
}

// ----------------------------------------
// Enable or disable microphone processing
/*
Description:
Enable/disable microphone processing logically, without physically disconnecting the microphone
Parameters:
- currently n/a
Returns:
- currently n/a
Notes:
- currently n/a
*/
// ----------------------------------------
esp_err_t audio_service_set_mute(bool mute) {
    // Enable/disable microphone here
    return ESP_OK;
}

// ############################################################################################## //
// RTOS FUNCTIONS
// ############################################################################################## //
// ----------------------------------------
// Create Audio Service Task
/*
Description:
Create the RTOS task for audio acquisition and processing using private functions
Parameters:
- currently n/a
Returns:
- currently n/a
Notes:
- currently n/a
*/
// ----------------------------------------
static void audio_service_task(void *arg);

// ----------------------------------------
// Start Audio Service
/*
Description:
Start the audio-processing task
Parameters:
- currently n/a
Returns:
- currently n/a
Notes:
- currently n/a
*/
// ----------------------------------------
esp_err_t audio_service_start(void) {
    // Start audio service operations here
    return ESP_OK;
}