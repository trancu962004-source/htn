idf_component_register(
    SRCS
        "src/app_main.c"
        "src/ble_scan.c"
        "src/gateway_handler.c"
        "src/node_store.c"
        "src/packet.c"
        "src/uart_bridge.c"
    INCLUDE_DIRS
        "include"
    REQUIRES nvs_flash bt driver
)
