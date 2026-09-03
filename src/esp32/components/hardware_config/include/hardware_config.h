#pragma once
#include "driver/gpio.h"

// ------------------------------
// Hardware Configuration for Edge Room Controller
/*
Description:
This header file defines the hardware configuration for the Edge Room Controller, including GPIO pin assignments for various
peripherals such as the INMP441 microphone, WS2812B RGB LED, BME280 environmental sensor, and manual control switches.
Notes:
- The GPIO pin assignments are defined using the ESP32 GPIO numbering scheme.
- The configuration is intended to be used across the project to ensure consistent hardware access.
*/
// ------------------------------

// INMP441 - I2S - Microphone
#define MIC_BCLK_GPIO GPIO_NUM_4
#define MIC_WS_GPIO GPIO_NUM_5
#define MIC_DATA_GPIO GPIO_NUM_6

// WS2812B - RGB LED
#define LED_DATA_GPIO GPIO_NUM_7

// BME280 - I2C - Environmental Sensor
/*
Breakout Board Pinout from top to bottom:
VCC, GND, SCL, SDA, CS, SDO
*/
#define I2C_SDA_GPIO GPIO_NUM_8
#define I2C_SCL_GPIO GPIO_NUM_9

// Manual Controls - Toggle Switches
#define MIC_MUTE_GPIO GPIO_NUM_10
#define CLOUD_ENABLE_GPIO GPIO_NUM_11