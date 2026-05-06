#pragma once

#include <stdbool.h>
#include "esp_err.h"
#include "vital_packet.h"

esp_err_t ble_init(void);
bool ble_is_synced(void);
void ble_set_packet(const vital_adv_pkt_t *pkt);
void ble_start_advertising(void);
