idf_component_register(
    SRCS "system_manager.c"
    INCLUDE_DIRS "include"
    REQUIRES i2c_bus bme280 environment_service
)