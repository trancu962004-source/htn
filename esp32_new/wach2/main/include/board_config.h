#pragma once

#include "driver/i2c.h"

/* =========================
 * I2C + MAX30100
 * ========================= */
#define I2C_PORT                I2C_NUM_0
#define I2C_SDA_GPIO            4
#define I2C_SCL_GPIO            3
#define I2C_FREQ_HZ             50000

#define REPORT_INTERVAL_MS      1000
#define SPO2_INTERVAL_MS        5000

#define HR_MIN_BPM              45.0f
#define HR_MAX_BPM              180.0f

#define SATURATION_LIMIT        65000U
#define MIN_VALID_IR            1000U
#define MIN_VALID_RED           1000U

/* =========================
 * BLE packet format
 * ========================= */
#define WING_ID                 'A'
#define ROOM_ID                 1
#define BED_ID                  2
#define PATIENT_ID              4
#define NODE_TYPE_VITAL         1

#define BLE_COMPANY_ID_LOW      0xFF
#define BLE_COMPANY_ID_HIGH     0xFF
#define PACKET_MAGIC            0xA5
#define DEVICE_NAME             "WATCH_A_R01_B02"
#define ADV_UPDATE_INTERVAL_MS  1000
#define SEQ_START_VALUE         100
