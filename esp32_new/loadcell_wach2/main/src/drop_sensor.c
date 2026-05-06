#include "drop_sensor.h"
#include "board_config.h"

#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "DROP_SENSOR";

static volatile uint32_t s_drop_total = 0;
static volatile uint32_t s_drop_count_window = 0;
static volatile int64_t s_last_drop_us = 0;

static uint32_t s_last_drops_per_min = 0;
static TickType_t s_last_rate_tick = 0;

static portMUX_TYPE s_drop_mux = portMUX_INITIALIZER_UNLOCKED;

static void IRAM_ATTR drop_isr_handler(void *arg)
{
    (void)arg;

    int64_t now_us = esp_timer_get_time();

    if ((now_us - s_last_drop_us) < MIN_DROP_GAP_US) {
        return;
    }

    s_last_drop_us = now_us;

    portENTER_CRITICAL_ISR(&s_drop_mux);
    s_drop_total++;
    s_drop_count_window++;
    portEXIT_CRITICAL_ISR(&s_drop_mux);
}

esp_err_t drop_sensor_init(void)
{
    gpio_config_t conf = {
        .pin_bit_mask = 1ULL << IR_DROP_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&conf), TAG, "drop sensor init failed");
    ESP_RETURN_ON_ERROR(gpio_install_isr_service(0), TAG, "gpio_install_isr_service failed");
    ESP_RETURN_ON_ERROR(gpio_isr_handler_add(IR_DROP_GPIO, drop_isr_handler, NULL), TAG,
                        "gpio_isr_handler_add failed");

    s_last_rate_tick = xTaskGetTickCount();
    return ESP_OK;
}

uint16_t drop_sensor_calc_drops_per_min(void)
{
    TickType_t now_tick = xTaskGetTickCount();
    TickType_t elapsed_tick = now_tick - s_last_rate_tick;

    if (elapsed_tick < pdMS_TO_TICKS(DROP_REPORT_INTERVAL_MS)) {
        return (uint16_t)s_last_drops_per_min;
    }

    uint32_t count_10s = 0;
    uint32_t total = 0;

    portENTER_CRITICAL(&s_drop_mux);
    count_10s = s_drop_count_window;
    s_drop_count_window = 0;
    total = s_drop_total;
    portEXIT_CRITICAL(&s_drop_mux);

    s_last_rate_tick = now_tick;
    s_last_drops_per_min = count_10s * 6;

    if (s_last_drops_per_min > 65535UL) {
        s_last_drops_per_min = 65535UL;
    }

    ESP_LOGI(TAG,
             "DROP | 10s_count=%" PRIu32 " dpm=%" PRIu32 " total=%" PRIu32,
             count_10s, s_last_drops_per_min, total);

    return (uint16_t)s_last_drops_per_min;
}
