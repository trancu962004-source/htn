#include "hx711.h"
#include "board_config.h"

#include <inttypes.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_rom_sys.h"

static const char *TAG = "HX711";
static int32_t s_hx711_offset = 0;

esp_err_t hx711_gpio_init(void)
{
    gpio_config_t in_conf = {
        .pin_bit_mask = 1ULL << HX711_DOUT,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&in_conf), TAG, "hx711 dout init failed");

    gpio_config_t out_conf = {
        .pin_bit_mask = 1ULL << HX711_SCK,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&out_conf), TAG, "hx711 sck init failed");

    ESP_RETURN_ON_ERROR(gpio_set_level(HX711_SCK, 0), TAG, "set HX711_SCK failed");
    return ESP_OK;
}

static bool hx711_wait_ready(uint32_t timeout_ms)
{
    uint32_t count = timeout_ms * 100;

    while (gpio_get_level(HX711_DOUT) == 1 && count > 0) {
        esp_rom_delay_us(10);
        count--;
    }

    return (count > 0);
}

esp_err_t hx711_read_raw(int32_t *out_raw)
{
    if (out_raw == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (!hx711_wait_ready(HX711_READY_TIMEOUT_MS)) {
        ESP_LOGE(TAG, "HX711 timeout waiting ready");
        return ESP_ERR_TIMEOUT;
    }

    int32_t data = 0;

    for (int i = 0; i < 24; i++) {
        gpio_set_level(HX711_SCK, 1);
        esp_rom_delay_us(2);
        data = (data << 1) | gpio_get_level(HX711_DOUT);
        gpio_set_level(HX711_SCK, 0);
        esp_rom_delay_us(2);
    }

    gpio_set_level(HX711_SCK, 1);
    esp_rom_delay_us(2);
    gpio_set_level(HX711_SCK, 0);
    esp_rom_delay_us(2);

    if (data & 0x800000) {
        data |= ~0xFFFFFF;
    }

    *out_raw = data;
    return ESP_OK;
}

esp_err_t hx711_read_average(uint32_t samples, int32_t *out_avg)
{
    if (out_avg == NULL || samples == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    int64_t sum = 0;
    uint32_t ok_count = 0;

    for (uint32_t i = 0; i < samples; i++) {
        int32_t raw = 0;
        if (hx711_read_raw(&raw) == ESP_OK) {
            sum += raw;
            ok_count++;
        }
        vTaskDelay(pdMS_TO_TICKS(HX711_SAMPLE_DELAY_MS));
    }

    if (ok_count == 0) {
        return ESP_FAIL;
    }

    *out_avg = (int32_t)(sum / (int64_t)ok_count);
    return ESP_OK;
}

esp_err_t hx711_tare(uint32_t samples)
{
    int32_t avg = 0;
    ESP_RETURN_ON_ERROR(hx711_read_average(samples, &avg), TAG, "tare failed");
    s_hx711_offset = avg;
    ESP_LOGI(TAG, "Tare done, offset=%" PRId32, s_hx711_offset);
    return ESP_OK;
}

int32_t hx711_get_offset(void)
{
    return s_hx711_offset;
}
