idf_component_register(
    SRCS
        "src/app_main.c"
        "src/app_state.c"
        "src/ble_scan.c"
        "src/keypad.c"
        "src/lcd_i2c.c"
        "src/lcd_task.c"
        "src/packet.c"
    INCLUDE_DIRS
        "include"
    REQUIRES nvs_flash bt driver
)
