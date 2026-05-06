#pragma once

#include <stdint.h>
#include "esp_err.h"

esp_err_t drop_sensor_init(void);
uint16_t drop_sensor_calc_drops_per_min(void);
