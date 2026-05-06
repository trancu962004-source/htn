#pragma once

#include "driver/gpio.h"
#include "driver/i2c.h"

/* =========================
 * Project tag
 * ========================= */
#define APP_TAG                  "GW_LCD_KEYPAD"

/* =========================
 * LCD 20x4 I2C
 * ========================= */
#define I2C_PORT                 I2C_NUM_0
#define I2C_SDA_GPIO             GPIO_NUM_3
#define I2C_SCL_GPIO             GPIO_NUM_2
#define I2C_FREQ_HZ              100000
#define LCD_I2C_ADDR             0x27

#define LCD_BACKLIGHT            0x08
#define LCD_ENABLE               0x04
#define LCD_RS                   0x01

/* =========================
 * Keypad 4x4
 * ========================= */
#define KP_R1                    GPIO_NUM_5
#define KP_R2                    GPIO_NUM_6
#define KP_R3                    GPIO_NUM_7
#define KP_R4                    GPIO_NUM_8

#define KP_C1                    GPIO_NUM_9
#define KP_C2                    GPIO_NUM_10
#define KP_C3                    GPIO_NUM_20
#define KP_C4                    GPIO_NUM_21

/* =========================
 * BLE packet config
 * ========================= */
#define COMPANY_ID_LOW           0xFF
#define COMPANY_ID_HIGH          0xFF
#define PACKET_MAGIC             0xA5
#define NODE_TYPE_VITAL          1
#define NODE_TYPE_LOADCELL       2

#define DATA_TIMEOUT_MS          5000
#define LCD_REFRESH_MS           250
