#include "bme280.h"

#include <stdint.h>

#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c_master.h"

#include "i2c_bus.h"

// --------------------------------------------------------------------------------------------------------------------------
// BME280.c
/*
Description:
Contains the implementation of functions to interface with the BME280 environmental sensor over I2C.
Notes:
- Conversion formulas for temperature, pressure, and humidity are based on the BME280 datasheet.
- Compensation functions use floating point due to periodic task being long enough to allow for floating point operations without significant performance impact.
*/
// --------------------------------------------------------------------------------------------------------------------------

// ------------------------------
// BME280 Sensor Constants
// ------------------------------
#define BME280_CHIP_ID 0x60
#define BME280_CHIP_ID_REG 0xD0
#define BME280_I2C_ADDRESS 0x76

#define BME280_I2C_FREQ_HZ 100000
#define BME280_I2C_TIMEOUT_MS 100

#define BME280_CTRL_HUM_REG 0xF2
#define BME280_CTRL_MEAS_REG 0xF4

#define BME280_CTRL_HUM_VALUE 0x01  // Humidity oversampling x1
#define BME280_CTRL_MEAS_VALUE 0x27 // Temperature and Pressure oversampling x1, Normal mode

// ------------------------------
// BME280 Sensor Handle and Calibration Data
// ------------------------------
static const char *TAG = "bme280";                      // Tag for logging
static i2c_master_dev_handle_t bme280_handle = NULL;    // Handle for the BME280 device
static int32_t t_fine; // Variable to hold the fine temperature value for compensation calculations

// --------------------------------
// BME280 Calibration Data Structure
/*
Description:
This structure holds the calibration data read from the BME280 sensor, which is used to convert raw sensor readings into actual temperature, pressure, and humidity values.
Notes:
- The calibration data is stored in specific registers of the BME280 sensor and must be read during initialization.
- The calibration parameters are used in the compensation formulas provided in the BME280 datasheet to calculate the actual sensor readings from the raw data.
- The structure includes parameters for temperature, pressure, and humidity calibration.
*/
// --------------------------------
typedef struct {
    // Temperature calibration parameters
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
    // Pressure calibration parameters
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
    // Humidity calibration parameters
    uint8_t dig_H1;
    int16_t dig_H2;
    uint8_t dig_H3;
    int16_t dig_H4;
    int16_t dig_H5;
    int8_t dig_H6;
} bme280_calib_t;
static bme280_calib_t calib; // Calibration data structure instance

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
- The function reads the calibration data from registers 0x88 to 0xA1 and 0xE1 to 0xE7, which contain the temperature, pressure, and humidity calibration parameters.
- Example: calib.dig_T1 takes value from 0x88 and 0x89 by combining them into a 16-bit unsigned integer.
    - Left shift the first byte by 8 bits and bitwise OR with the second byte.
    - This combines two bytes into a single 16-bit value.
*/
// --------------------------------
static esp_err_t bme280_read_calibration(void) {
    uint8_t calib1[26]; // Buffer for calibration data from registers 0x88 to 0xA1
    uint8_t calib2[7];  // Buffer for calibration data from registers 0xE1 to 0xE7

    // Read calibration data from registers 0x88 to 0xA1
    esp_err_t err = bme280_read_registers(0x88, calib1, sizeof(calib1));
    if (err != ESP_OK) {
        return err;
    }
    // Read calibration data from registers 0xE1 to 0xE7
    err = bme280_read_registers(0xE1, calib2, sizeof(calib2));
    if (err != ESP_OK) {
        return err;
    }
    // Temperature calibration parameters
    calib.dig_T1 = (uint16_t)(calib1[1] << 8 | calib1[0]);
    calib.dig_T2 = (int16_t)(calib1[3] << 8 | calib1[2]);
    calib.dig_T3 = (int16_t)(calib1[5] << 8 | calib1[4]);
    // Pressure calibration parameters
    calib.dig_P1 = (uint16_t)(calib1[7] << 8 | calib1[6]);
    calib.dig_P2 = (int16_t)(calib1[9] << 8 | calib1[8]);
    calib.dig_P3 = (int16_t)(calib1[11] << 8 | calib1[10]);
    calib.dig_P4 = (int16_t)(calib1[13] << 8 | calib1[12]);
    calib.dig_P5 = (int16_t)(calib1[15] << 8 | calib1[14]);
    calib.dig_P6 = (int16_t)(calib1[17] << 8 | calib1[16]);
    calib.dig_P7 = (int16_t)(calib1[19] << 8 | calib1[18]);
    calib.dig_P8 = (int16_t)(calib1[21] << 8 | calib1[20]);
    calib.dig_P9 = (int16_t)(calib1[23] << 8 | calib1[22]);
    // Humidity calibration parameters
    // Notes: H4 and H5 are stored in a non-standard way across two registers, requiring bit manipulation to extract the correct values.
    calib.dig_H1 = calib1[25];
    calib.dig_H2 = (int16_t)(calib2[1] << 8 | calib2[0]);
    calib.dig_H3 = calib2[2];
    calib.dig_H4 = (int16_t)((calib2[3] << 4) | (calib2[4] & 0x0F));
    calib.dig_H5 = (int16_t)((calib2[5] << 4) | (calib2[4] >> 4));
    calib.dig_H6 = (int8_t)calib2[6];

    return ESP_OK;
}

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
6. Reads the calibration data from the BME280 for accurate sensor readings.
Parameters:
- None
Returns:
- ESP_OK on success, or an appropriate error code on failure.  
Notes:
- bus_handle is the handle to the I2C bus obtained from i2c_bus_get_handle().
- ESP_LOGE and ESP_LOGI are used for logging error and informational messages, respectively.
- i2c_master_tranmit_receive is a function that performs an I2C transaction to read data between the esp32 and the BME280 device.
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

    // Read and store the calibration data from the BME280
    err = bme280_read_calibration();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read calibration data: %s", esp_err_to_name(err));
        return err;
    }

    return ESP_OK;
}

// ---------------------------------
// BME280 Compensate Temperature Function
/*
Description:
This function compensates the raw BME280 temperature ADC value to obtain temperature in degrees Celsius using the sensor's factory calibration coefficients.
Parameters:
- adc_T: The raw temperature reading from the BME280 sensor (as a 32-bit signed integer).
Returns:
- The compensated temperature in degrees Celsius (as a float).
Notes:
- var1 and var2 are intermediate values defined by Bosch's compensation algorithm.
- Their sum is stored as t_fine, an intermediate temperature-dependent value required by the pressure and humidity compensation algorithms.
- The final division by 5120.0 is part of Bosch's floating-point compensation formula and produces temperature in degrees Celsius.
*/
// ---------------------------------
static float bme280_compensate_temperature(int32_t adc_T) {
    // Calculate var1 using the calibration parameters and the raw temperature reading
    float var1 = (((float)adc_T) / 16384.0f - ((float)calib.dig_T1) / 1024.0f) * ((float)calib.dig_T2);
    // Calculate var2 using the calibration parameters and the raw temperature reading
    float var2 = ((((float)adc_T) / 131072.0f - ((float)calib.dig_T1) / 8192.0f) * (((float)adc_T) / 131072.0f - ((float)calib.dig_T1) / 8192.0f)) * ((float)calib.dig_T3);
    // Store the fine temperature value for pressure and humidity compensation
    t_fine = (int32_t)(var1 + var2);
    // Calculate the actual temperature in degrees Celsius
    float temp = (var1 + var2) / 5120.0f;
    return temp;
}

// ---------------------------------
// BME280 Compensate Pressure Function
/*
Description:
This function compensates the raw BME280 pressure ADC value using the sensor's factory pressure calibration coefficients and t_fine.
Parameters:
- adc_P: The raw pressure reading from the BME280 sensor (as a 32-bit signed integer).
Returns:
- The compensated pressure in hectopascals (as a float).
Notes:
- Pressure compensation depends on t_fine, which must first be updated by the temperature compensation function for the same measurement cycle.
- var1 and var2 are intermediate values defined by Bosch's compensation algorithm.
- The Bosch compensation calculation produces pressure in pascals (Pa). This function divides that result by 100 before returning it as hPa.
- The var1 == 0 check prevents division by zero.
*/
// ---------------------------------
static float bme280_compensate_pressure(int32_t adc_P) {
    // Calculate var1 and var2 using the calibration parameters and the fine temperature value (t_fine)
    float var1 = ((float)t_fine / 2.0f) - 64000.0f;
    float var2 = var1 * var1 * ((float)calib.dig_P6) / 32768.0f;
    var2 = var2 + var1 * ((float)calib.dig_P5) * 2.0f;
    var2 = (var2 / 4.0f) + (((float)calib.dig_P4) * 65536.0f);
    var1 = (((float)calib.dig_P3) * var1 * var1 / 524288.0f + ((float)calib.dig_P2) * var1) / 524288.0f;
    var1 = (1.0f + var1 / 32768.0f) * ((float)calib.dig_P1);
    // Avoid division by zero
    if (var1 == 0.0f) {
        return 0;
    }
    // Apply Bosch compensation to calculate pressure in pascals (Pa)
    float pressure = 1048576.0f - (float)adc_P;
    pressure = (pressure - (var2 / 4096.0f)) * 6250.0f / var1;
    var1 = ((float)calib.dig_P9) * pressure * pressure / 2147483648.0f;
    var2 = pressure * ((float)calib.dig_P8) / 32768.0f;
    pressure = pressure + (var1 + var2 + ((float)calib.dig_P7)) / 16.0;
    // Convert pascals to hectopascals before returning
    return pressure / 100.0;
}

// ---------------------------------
// BME280 Compensate Humidity Function
/*
Description:
This function compensates the raw BME280 humidity ADC value to obtain relative humidity in percent (%RH).
Parameters:
- adc_H: The raw humidity reading from the BME280 sensor (as a 32-bit signed integer).
Returns:
- The compensated humidity in percentage (as a float).
Notes:
- Humidity compensation uses the factory humidity calibration coefficients and t_fine from the temperature compensation calculation.
- The compensation expression is defined by Bosch for the BME280.
- The final result is clamped to the physical range of 0% to 100% RH.
*/
// ---------------------------------
static float bme280_compensate_humidity(int32_t adc_H) {
    // Calculate the actual humidity in percentage using the raw humidity reading and the fine temperature value (t_fine)
    float humidity = (float)t_fine - 76800.0f;
    humidity = (adc_H - (((float)calib.dig_H4) * 64.0f + ((float)calib.dig_H5) / 16384.0f * humidity)) *
               (((float)calib.dig_H2) / 65536.0f * (1.0f + ((float)calib.dig_H6) / 67108864.0f * humidity *
               (1.0f + ((float)calib.dig_H3) / 67108864.0f * humidity)));
    // Apply the final humidity compensation term
    humidity = humidity * (1.0f - ((float)calib.dig_H1) * humidity / 524288.0f);
    // Clamp the compensated relative humidity to the physical range of 0-100%
    if (humidity > 100.0f) {
        humidity = 100.0f;
    } else if (humidity < 0.0f) {
        humidity = 0.0f;
    }
    return humidity;
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
    data->temperature_c = bme280_compensate_temperature(raw_temperature);
    data->pressure_hpa = bme280_compensate_pressure(raw_pressure);
    data->humidity_percent = bme280_compensate_humidity(raw_humidity);

    return ESP_OK;
}