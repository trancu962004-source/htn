#pragma once

#include "driver/gpio.h"

#define WING_ID                 'B'
#define ROOM_ID                 1
#define BED_ID                  2
#define PATIENT_ID              3
#define NODE_TYPE_LOADCELL      2

#define HX711_DOUT              GPIO_NUM_1
#define HX711_SCK               GPIO_NUM_0
#define IR_DROP_GPIO            GPIO_NUM_10

#define HX711_SAMPLES           10
#define HX711_SAMPLE_DELAY_MS   20
#define HX711_READY_TIMEOUT_MS  1000
#define HX711_STARTUP_DELAY_MS  1000
#define ADV_UPDATE_INTERVAL_MS  1000

#define MIN_DROP_GAP_US         180000
#define DROP_REPORT_INTERVAL_MS 10000

#define LOADCELL_SCALE_FACTOR   255.0f
#define NEGATIVE_CLAMP_ML       -2.0f

#define BLE_COMPANY_ID_LOW      0xFF
#define BLE_COMPANY_ID_HIGH     0xFF
#define PACKET_MAGIC            0xA5
#define DEVICE_NAME             "LOADCELL_B_R01_B03"

#define SEQ_START_VALUE         100
#define SEQ_END_VALUE           199
