#pragma once

#include "esp_err.h"
#include "packet.h"

#ifdef __cplusplus
extern "C" {
#endif

void uart_bridge_init(void);
void uart_bridge_send_vital(const vital_adv_pkt_t *pkt);
void uart_bridge_send_loadcell(const loadcell_adv_pkt_t *pkt);

#ifdef __cplusplus
}
#endif
