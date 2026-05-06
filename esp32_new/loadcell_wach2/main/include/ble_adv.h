#pragma once

#include <stdbool.h>
#include "esp_err.h"
#include "loadcell_packet.h"

esp_err_t ble_adv_init(const loadcell_adv_pkt_t *initial_packet);
void ble_adv_set_packet(const loadcell_adv_pkt_t *packet);
void ble_adv_start_advertising(void);
bool ble_adv_is_synced(void);
