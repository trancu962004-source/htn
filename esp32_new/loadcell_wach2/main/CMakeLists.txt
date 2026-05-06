idf_component_register(
    SRCS
        "src/app_main.c"
        "src/hx711.c"
        "src/drop_sensor.c"
        "src/loadcell_sensor.c"
        "src/loadcell_packet.c"
        "src/ble_adv.c"
    INCLUDE_DIRS
        "include"
    REQUIRES nvs_flash bt driver esp_timer
)
