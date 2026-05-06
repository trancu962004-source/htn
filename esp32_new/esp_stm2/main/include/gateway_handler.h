#pragma once

#include "packet.h"

#ifdef __cplusplus
extern "C" {
#endif

void handle_vital_packet(const vital_adv_pkt_t *pkt);
void handle_loadcell_packet(const loadcell_adv_pkt_t *pkt);

#ifdef __cplusplus
}
#endif
