#pragma once
#include "driver/gpio.h"

/*
#Edge Room Controller
Central Hardware Configuration
*/ 

// INMP441 - I2S - Microphone
#define MIC_BCLK_GPIO GPIO_NUM_4
#define MIC_WS_GPIO GPIO_NUM_5
#define MIC_DATA_GPIO GPIO_NUM_6

// WS2812B - RGB LED
#define LED_DATA_GPIO GPIO_NUM_7

// BME280 - I2C - Environmental Sensor
#define I2C_SDA_GPIO GPIO_NUM_8
#define I2C_SCL_GPIO GPIO_NUM_9

// Manual Controls - Toggle Switches
#define MIC_MUTE_GPIO GPIO_NUM_10
#define CLOUD_ENABLE_GPIO GPIO_NUM_11