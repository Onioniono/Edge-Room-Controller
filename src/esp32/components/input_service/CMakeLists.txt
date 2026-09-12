idf_component_register(
    SRCS "input_service.c"
    INCLUDE_DIRS "include"
    REQUIRES hardware_config esp_driver_gpio
)