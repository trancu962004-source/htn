#pragma once

#include <stdint.h>
#include <stddef.h>
#include "loadcell_types.h"

typedef struct __attribute__((packed)) {
    uint8_t  magic;
    uint8_t  wing_id;
    uint8_t  room_id;
    uint8_t  bed_id;
    uint8_t  patient_id;
    uint8_t  node_type;
    uint16_t volume_ml;
    uint16_t drops_per_min;
    uint8_t  seq;
    uint8_t  checksum;
} loadcell_adv_pkt_t;

uint8_t loadcell_calc_checksum(const uint8_t *data, size_t len);
void loadcell_packet_init(loadcell_adv_pkt_t *pkt);
void loadcell_packet_fill_from_reading(loadcell_adv_pkt_t *pkt, const loadcell_data_t *reading);
