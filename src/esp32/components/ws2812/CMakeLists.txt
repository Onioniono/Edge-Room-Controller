idf_component_register(
    SRCS "ws2812.c"
    INCLUDE_DIRS "include"
    REQUIRES hardware_config esp_driver_rmt
)