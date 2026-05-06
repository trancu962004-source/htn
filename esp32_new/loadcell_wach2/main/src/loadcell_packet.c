#include "loadcell_packet.h"
#include "board_config.h"

static uint8_t s_seq = SEQ_START_VALUE;

uint8_t loadcell_calc_checksum(const uint8_t *data, size_t len)
{
    uint8_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum ^= data[i];
    }
    return sum;
}

static uint8_t next_seq(void)
{
    uint8_t current = s_seq;

    if (s_seq >= SEQ_END_VALUE) {
        s_seq = SEQ_START_VALUE;
    } else {
        s_seq++;
    }

    return current;
}

void loadcell_packet_init(loadcell_adv_pkt_t *pkt)
{
    if (pkt == NULL) {
        return;
    }

    pkt->magic = PACKET_MAGIC;
    pkt->wing_id = (uint8_t)WING_ID;
    pkt->room_id = ROOM_ID;
    pkt->bed_id = BED_ID;
    pkt->patient_id = PATIENT_ID;
    pkt->node_type = NODE_TYPE_LOADCELL;
    pkt->volume_ml = 0;
    pkt->drops_per_min = 0;
    pkt->seq = SEQ_START_VALUE;
    pkt->checksum = 0;
}

void loadcell_packet_fill_from_reading(loadcell_adv_pkt_t *pkt, const loadcell_data_t *reading)
{
    if (pkt == NULL || reading == NULL) {
        return;
    }

    pkt->magic = PACKET_MAGIC;
    pkt->wing_id = (uint8_t)WING_ID;
    pkt->room_id = ROOM_ID;
    pkt->bed_id = BED_ID;
    pkt->patient_id = PATIENT_ID;
    pkt->node_type = NODE_TYPE_LOADCELL;
    pkt->volume_ml = reading->volume_ml;
    pkt->drops_per_min = reading->drops_per_min;
    pkt->seq = next_seq();
    pkt->checksum = loadcell_calc_checksum((const uint8_t *)pkt, sizeof(*pkt) - 1);
}
