#include "audio_service.h"
#include "esp_heap_caps.h" // Include ESP-IDF heap capabilities header for memory management
#include "i2s_bus.h"

/************************* 
Purpose: 
Continuously acquire audio, manage application buffers, prepare samples, coordinate speech recognition
*************************/

#define AUDIO_RAW_FRAME_COUNT 256                                       // # of raw audio frames to acquire before processing
#define AUDIO_RAW_BUFFER_SIZE (AUDIO_RAW_FRAME_COUNT*2*sizeof(int32_t)) // Size of the raw audio buffer in bytes

static const char *TAG = "AUDIO_SERVICE";                               // Tag for logging
static int32_t *audio_raw_buffer = NULL;                                // Pointer to the raw audio buffer for storing acquired audio samples

static bool audio_initialized = false;                                  // Flag to indicate audio service initialization status
static bool audio_started = false;                                      // Flag to indicate audio service start status
static bool audio_muted = false;                                        // Flag to indicate microphone mute status

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
- None
Returns:
- ESP_OK if successful, or an appropriate error code if failed.
Notes:
- Does not have ESP-SR resources initialized yet; those will be initialized later when needed.
- Dynamically allocates memory for the raw audio buffer and initializes the I2S bus for audio acquisition.
- If initialization fails, it cleans up any allocated resources to prevent memory leaks.
*/
// ----------------------------------------
esp_err_t audio_service_init(void) {
    // Prevent repreated initialization
    if (audio_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    // Initialize I2S RX Peripheral & DMA
    esp_err_t err = i2s_bus_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2S bus: %s", esp_err_to_name(err));
        return err;
    }
    // Allocate memory (dynamically)for raw incoming audio samples
    audio_raw_buffer = heap_caps_malloc(AUDIO_RAW_BUFFER_SIZE, MALLOC_CAP_8BIT);
    if (audio_raw_buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate audio buffer");
        i2s_bus_deinit(); // Clean up I2S bus if memory allocation fails
        return ESP_ERR_NO_MEM;
    }
    
    // ESP-SR Processing resources WIP and will be initialized later.

    // Initialize audio service state flags
    audio_muted = false;
    audio_started = false;
    audio_initialized = true; // Flag audio service as initialized

    ESP_LOGI(TAG, "Audio Service Initialized");
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