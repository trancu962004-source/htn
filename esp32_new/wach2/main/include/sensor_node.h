#pragma once

#include "esp_err.h"
#include "vital_packet.h"
esp_err_t sensor_node_init(void);
void sensor_task(void *arg);
// lấy dữ liệu sinh hiệu mới nhất.
void sensor_get_latest(vital_data_t *vd);
