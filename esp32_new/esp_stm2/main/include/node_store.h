#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool used;
    uint8_t wing_id;
    uint8_t room_id;
    uint8_t bed_id;
    uint8_t patient_id;
    uint8_t node_type;
    uint8_t last_seq;
    uint32_t last_seen_ms;

    bool has_vital;
    uint16_t hr;
    uint16_t spo2;
    uint16_t temp_x100;
    uint8_t status;
    uint8_t battery;

    bool has_loadcell;
    uint16_t volume_ml;
    uint16_t drops_per_min;
} node_state_t;

void node_store_init(void);
int node_store_find_or_alloc(uint8_t wing_id, uint8_t room_id, uint8_t bed_id,
                             uint8_t patient_id, uint8_t node_type);
node_state_t *node_store_get(int idx);
void node_monitor_task(void *arg);

#ifdef __cplusplus
}
#endif
