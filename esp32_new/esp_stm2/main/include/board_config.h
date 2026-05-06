#pragma once

#include "driver/uart.h"

#define MY_ROOM_ID              1
#define MY_WING_ID              'A'
#define MANAGED_BED_START       1
#define MANAGED_BED_END         6

#define NODE_TIMEOUT_MS         5000
#define MAX_NODES               32

#define COMPANY_ID_LOW          0xFF
#define COMPANY_ID_HIGH         0xFF
#define PACKET_MAGIC            0xA5

#define NODE_TYPE_VITAL         1
#define NODE_TYPE_LOADCELL      2

#define UART_PORT_NUM           UART_NUM_1
#define UART_TX_PIN             17
#define UART_RX_PIN             16
#define UART_BAUD_RATE          115200
#define UART_BUF_SIZE           1024
