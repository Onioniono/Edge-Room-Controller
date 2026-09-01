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
/*
Description:
This function writes a single byte value to a specified register of the BME280 sensor over I2C.
Parameters:
- reg: The register address to which the value will be written.
- value: The byte value to write to the specified register.
Returns:
- ESP_OK on success, or an appropriate error code on failure.
Notes:
- The function constructs a buffer containing the register address followed by the value to be written.
- It uses the i2c_master_transmit function to send the buffer to the BME280 device over the I2C bus.
*/
// --------------------------------
static esp_err_t bme280_write_register(uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = {reg, value};
    return i2c_master_transmit(bme280_handle, buffer, sizeof(buffer), BME280_I2C_TIMEOUT_MS);
}

// ---------------------------------
// BME280 Read Function
/*
Description:
This function reads a specified number of bytes from a starting register of the BME280 sensor over I2C.
Parameters:
- start_reg: The starting register address from which to read data.
- buffer: A pointer to a buffer where the read data will be stored.
- length: The number of bytes to read from the sensor.
Returns:
- ESP_OK on success, or an appropriate error code on failure.
Notes:
- The function uses the i2c_master_transmit_receive function to perform a combined write (to specify the starting register) 
and read (to retrieve the data) operation over the I2C bus.
*/
// ---------------------------------
static esp_err_t bme280_read_registers(uint8_t start_reg, uint8_t *buffer, size_t length) {
    return i2c_master_transmit_receive(bme280_handle, &start_reg, sizeof(start_reg), buffer, length, BME280_I2C_TIMEOUT_MS);
}

// --------------------------------
// BME280 Read Calibration Data Function
/*
Description:
This function reads the calibration data from the BME280 sensor over I2C.
Parameters:
- None
Returns:
- ESP_OK on success, or an appropriate error code on failure.
Notes:
- The calibration data is stored in specific registers of the BME280 sensor.
*/
// --------------------------------


// --------------------------------
// BME280 Initialization Function
/*
Description:
This function initializes the BME280 sensor by performing the following steps:
1. Obtains the I2C bus handle using `i2c_bus_get_handle()`.
2. Configures the BME280 device with the appropriate I2C address and settings.
3. Adds the BME280 device to the I2C bus using `i2c_master_bus_add_device()`.
4. Reads the chip ID from the BME280 to verify that the device is present and functioning correctly.
5. Configures the BME280 measurement settings for humidity, temperature, and pressure.
Notes:
- bus_handle is the handle to the I2C bus obtained from i2c_bus_get_handle().
- ESP_LOGE and ESP_LOGI are used for logging error and informational messages, respectively.
- i2c_master_tranmit_receive is a function that performs an I2C transaction to read data between the esp32 and the BME280 device.
- Example: i2c_master_transmit_receive(handle, &register_address, sizeof(register_address), &chip_id, sizeof(chip_id), BME280_I2C_TIMEOUT_MS)
- returns ESP_OK on success or an appropriate error code on failure.
- Initialization seems condense due to esp error handling and logging, but it is necessary to ensure proper communication with the BME280 sensor.
*/
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
/*
Description:
This function reads the temperature, pressure, and humidity data from the BME280 sensor and stores it in a provided structure.
Parameters:
- data: A pointer to a bme280_data_t structure where the read sensor data will be stored.
Returns:
- ESP_OK on success, or an appropriate error code on failure.
Notes:
- The function first checks if the provided data pointer is NULL and returns an error if it is.
- It reads raw data from the BME280 sensor starting from register 0xF7, which contains the pressure, temperature, and humidity data.
- The raw data is then processed to extract the individual pressure, temperature, and humidity values.
- The raw values are combined and shifted to form the actual sensor readings.
*/
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
    // Shift and combine the raw data bytes to form the raw pressure, temperature, and humidity values
    uint32_t raw_pressure = ((uint32_t)raw_data[0] << 12) | ((uint32_t)raw_data[1] << 4) | ((uint32_t)(raw_data[2] >> 4));
    uint32_t raw_temperature = ((uint32_t)raw_data[3] << 12) | ((uint32_t)raw_data[4] << 4) | ((uint32_t)(raw_data[5] >> 4));
    uint32_t raw_humidity = ((uint32_t)raw_data[6] << 8) | (uint32_t)raw_data[7];
    // Convert raw values to actual temperature, pressure, and humidity using calibration data
    // placeholder

    return ESP_OK;
}