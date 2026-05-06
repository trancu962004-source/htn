#pragma once

#include <stdint.h>
#include "esp_err.h"

esp_err_t hx711_gpio_init(void);
esp_err_t hx711_read_raw(int32_t *out_raw);
esp_err_t hx711_read_average(uint32_t samples, int32_t *out_avg);
esp_err_t hx711_tare(uint32_t samples);
int32_t hx711_get_offset(void);
