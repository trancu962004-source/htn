#pragma once

#include "esp_err.h"
#include "loadcell_types.h"

esp_err_t loadcell_sensor_init(void);
esp_err_t loadcell_read(loadcell_data_t *out);
