#pragma once

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t  magic;
    uint8_t  wing_id;
    uint8_t  room_id;
    uint8_t  bed_id;
    uint8_t  patient_id;
    uint8_t  node_type;
    uint16_t hr;
    uint16_t spo2;
    uint16_t temp_x100;
    uint8_t  status;
    uint8_t  battery;
    uint8_t  seq;
    uint8_t  checksum;
} vital_adv_pkt_t;

typedef struct {
    uint16_t hr;
    uint16_t spo2;
    uint16_t temp_x100;
    uint8_t status;
    uint8_t battery;
} vital_data_t;

void vital_packet_init(vital_adv_pkt_t *pkt);
void packet_fill_from_sensor(vital_adv_pkt_t *pkt, const vital_data_t *vd);
