#include "bme280.h"

#include <stdint.h>

#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c_master.h"

#include "i2c_bus.h"

#define BME280_CHIP_ID 0x60
#define BME280_CHIP_ID_REG 0xD0
#define BME280_I2C_ADDRESS 0x76

#define BME280_I2C_FREQ_HZ 100000
#define BME280_I2C_TIMEOUT_MS 100

#define BME280_CTRL_HUM_REG 0xF2
#define BME280_CTRL_MEAS_REG 0xF4

#define BME280_CTRL_HUM_VALUE 0x01  // Humidity oversampling x1
#define BME280_CTRL_MEAS_VALUE 0x27 // Temperature and Pressure oversampling x1, Normal mode

static const char *TAG = "bme280";  // Tag for logging
static i2c_master_dev_handle_t bme280_handle = NULL;  // Handle for the BME280 device

// --------------------------------
// BME280 Write Function
// --------------------------------
static esp_err_t bme280_write_register(uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = {reg, value};
    return i2c_master_transmit(bme280_handle, buffer, sizeof(buffer), BME280_I2C_TIMEOUT_MS);
}

// ---------------------------------
// BME280 Read Function
// ---------------------------------
static esp_err_t bme280_read_registers(uint8_t start_reg, uint8_t *buffer, size_t length) {
    return i2c_master_transmit_receive(bme280_handle, &start_reg, sizeof(start_reg), buffer, length, BME280_I2C_TIMEOUT_MS);
}

// --------------------------------
// BME280 Read Calibration Data Function
// --------------------------------


// --------------------------------
// BME280 Initialization Function
// --------------------------------
esp_err_t bme280_init(void) {
    // Get the I2C bus handle
    i2c_master_bus_handle_t bus_handle = i2c_bus_get_handle();
    if (bus_handle == NULL) {
        ESP_LOGE(TAG, "I2C bus not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    // Configure the BME280 device
    i2c_device_config_t device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = BME280_I2C_ADDRESS,
        .scl_speed_hz = BME280_I2C_FREQ_HZ,
    };
    // Add the BME280 device to the I2C bus
    esp_err_t err = i2c_master_bus_add_device(bus_handle, &device_config, &bme280_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add BME280 to I2C bus: %s", esp_err_to_name(err));
        return err;
    }
    // Read the chip ID from the BME280
    uint8_t register_address = BME280_CHIP_ID_REG;
    uint8_t chip_id = 0;
    err = i2c_master_transmit_receive(bme280_handle, &register_address, sizeof(register_address), &chip_id, sizeof(chip_id), BME280_I2C_TIMEOUT_MS);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read chip ID: %s", esp_err_to_name(err));
        return err;
    }
    if (chip_id != BME280_CHIP_ID) {
        ESP_LOGE(TAG, "Unexpected chip ID: 0x%02X", chip_id);
        return ESP_ERR_INVALID_RESPONSE;
    }
    ESP_LOGI(TAG, "BME280 verified with chip ID 0x%02X", chip_id);

    // Configure the BME280 measurement settings
    err = bme280_write_register(BME280_CTRL_HUM_REG, BME280_CTRL_HUM_VALUE);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure humidity");
        return err;
    }
    // Configure temperature and pressure measurement settings
    err = bme280_write_register(BME280_CTRL_MEAS_REG, BME280_CTRL_MEAS_VALUE);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure temperature and pressure");
        return err;
    }
    ESP_LOGI(TAG, "BME280 measurement configuration completed");

    return ESP_OK;
}

// ---------------------------------
// BME280 Read Sensor Data Function
// ---------------------------------
esp_err_t bme280_read(bme280_data_t *data) {
    if (data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    // Read raw data from the BME280
    uint8_t raw_data[8];
    esp_err_t err = bme280_read_registers(0xF7, raw_data, sizeof(raw_data));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read sensor data: %s", esp_err_to_name(err));
        return err;
    }
    // Convert raw data to temperature, pressure, and humidity
    // Note: The conversion formulas are based on the BME280 datasheet and calibration data
    uint32_t raw_pressure = ((uint32_t)raw_data[0] << 12) | ((uint32_t)raw_data[1] << 4) | ((uint32_t)(raw_data[2] >> 4));
    uint32_t raw_temperature = ((uint32_t)raw_data[3] << 12) | ((uint32_t)raw_data[4] << 4) | ((uint32_t)(raw_data[5] >> 4));
    uint32_t raw_humidity = ((uint32_t)raw_data[6] << 8) | (uint32_t)raw_data[7];

    return ESP_OK;
}