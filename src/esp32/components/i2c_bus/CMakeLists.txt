idf_component_register(
    SRCS "i2c_bus.c"
    INCLUDE_DIRS "include"
    REQUIRES hardware_config esp_driver_i2c
)