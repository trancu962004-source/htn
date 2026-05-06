#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

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

uint8_t calc_checksum(const uint8_t *data, uint16_t len);
bool parse_vital_packet(const uint8_t *mfg, uint8_t len, vital_adv_pkt_t *pkt);
bool parse_loadcell_packet(const uint8_t *mfg, uint8_t len, loadcell_adv_pkt_t *pkt);

#ifdef __cplusplus
}
#endif
