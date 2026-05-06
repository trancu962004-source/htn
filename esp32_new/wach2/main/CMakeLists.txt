idf_component_register(
    SRCS
        "src/app_main.c"
        "src/sensor_node.c"
        "src/pulseox.c"
        "src/vital_packet.c"
        "src/ble_adv.c"
        "src/max30100.c"
    INCLUDE_DIRS
        "include"
    REQUIRES nvs_flash bt driver
)