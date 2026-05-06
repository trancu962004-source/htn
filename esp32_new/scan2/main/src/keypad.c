#include "keypad.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_rom_sys.h"

#include "app_state.h"
#include "board_config.h"

static const char keypad_map[4][4] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

static const gpio_num_t row_pins[4] = {KP_R1, KP_R2, KP_R3, KP_R4};
static const gpio_num_t col_pins[4] = {KP_C1, KP_C2, KP_C3, KP_C4};

void keypad_init(void)
{
    gpio_config_t row_conf = {
        .pin_bit_mask = (1ULL << KP_R1) | (1ULL << KP_R2) | (1ULL << KP_R3) | (1ULL << KP_R4),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&row_conf));

    gpio_config_t col_conf = {
        .pin_bit_mask = (1ULL << KP_C1) | (1ULL << KP_C2) | (1ULL << KP_C3) | (1ULL << KP_C4),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&col_conf));

    for (int i = 0; i < 4; i++) {
        gpio_set_level(row_pins[i], 1);
    }
}

static char keypad_get_key(void)
{
    for (int r = 0; r < 4; r++) {
        for (int i = 0; i < 4; i++) {
            gpio_set_level(row_pins[i], 1);
        }

        gpio_set_level(row_pins[r], 0);
        esp_rom_delay_us(5);

        for (int c = 0; c < 4; c++) {
            if (gpio_get_level(col_pins[c]) == 0) {
                while (gpio_get_level(col_pins[c]) == 0) {
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
                return keypad_map[r][c];
            }
        }
    }
    return 0;
}

static void apply_input_value(void)
{
    if (s_input_len == 0) return;

    int val = atoi(s_input_buf);

    switch (s_input_mode) {
        case INPUT_W:
            if (val >= 1 && val <= 26) {
                s_sel_wing = 'A' + (val - 1);
            }
            break;

        case INPUT_R:
            if (val >= 1 && val <= 99) {
                s_sel_room = (uint8_t)val;
            }
            break;

        case INPUT_B:
            if (val >= 1 && val <= 99) {
                s_sel_bed = (uint8_t)val;
            }
            break;

        case INPUT_P:
            if (val >= 1 && val <= 99) {
                s_sel_patient = (uint8_t)val;
            }
            break;

        default:
            break;
    }

    memset(s_input_buf, 0, sizeof(s_input_buf));
    s_input_len = 0;
    s_input_mode = INPUT_NONE;
}

void keypad_task(void *arg)
{
    (void)arg;

    while (1) {
        char key = keypad_get_key();
        if (key != 0) {
            if (key == 'A') {
                s_input_mode = INPUT_W;
                s_input_len = 0;
                memset(s_input_buf, 0, sizeof(s_input_buf));
            } else if (key == 'B') {
                s_input_mode = INPUT_R;
                s_input_len = 0;
                memset(s_input_buf, 0, sizeof(s_input_buf));
            } else if (key == 'C') {
                s_input_mode = INPUT_B;
                s_input_len = 0;
                memset(s_input_buf, 0, sizeof(s_input_buf));
            } else if (key == 'D') {
                s_input_mode = INPUT_P;
                s_input_len = 0;
                memset(s_input_buf, 0, sizeof(s_input_buf));
            } else if (key == '*') {
                if (s_input_len > 0) {
                    s_input_len--;
                    s_input_buf[s_input_len] = '\0';
                }
            } else if (key == '#') {
                apply_input_value();
            } else if (isdigit((unsigned char)key)) {
                if (s_input_mode != INPUT_NONE && s_input_len < sizeof(s_input_buf) - 1) {
                    s_input_buf[s_input_len++] = key;
                    s_input_buf[s_input_len] = '\0';
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(40));
    }
}
