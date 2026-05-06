#include "vital_packet.h"
#include "board_config.h"

#include <string.h>

static uint8_t s_seq = SEQ_START_VALUE;

/* =========================
 * BLE helpers
 * ========================= */
static uint8_t calc_checksum(const uint8_t *data, uint16_t len)
{
    uint8_t sum = 0;
    for (uint16_t i = 0; i < len; i++) {
        sum ^= data[i];
    }
    return sum;
}

static uint8_t next_seq(void)
{
    uint8_t current = s_seq;
    if (s_seq >= 255) {
        s_seq = SEQ_START_VALUE;
    } else {
        s_seq++;
    }
    return current;
}

void vital_packet_init(vital_adv_pkt_t *pkt)
{
    memset(pkt, 0, sizeof(*pkt));
    pkt->magic = PACKET_MAGIC;
    pkt->wing_id = (uint8_t)WING_ID;
    pkt->room_id = ROOM_ID;
    pkt->bed_id = BED_ID;
    pkt->patient_id = PATIENT_ID;
    pkt->node_type = NODE_TYPE_VITAL;
}

void packet_fill_from_sensor(vital_adv_pkt_t *pkt, const vital_data_t *vd)
{
    pkt->magic = PACKET_MAGIC;
    pkt->wing_id = (uint8_t)WING_ID;
    pkt->room_id = ROOM_ID;
    pkt->bed_id = BED_ID;
    pkt->patient_id = PATIENT_ID;
    pkt->node_type = NODE_TYPE_VITAL;
    pkt->hr = vd->hr;
    pkt->spo2 = vd->spo2;
    pkt->temp_x100 = vd->temp_x100;
    pkt->status = vd->status;
    pkt->battery = vd->battery;
    pkt->seq = next_seq();
    pkt->checksum = calc_checksum((const uint8_t *)pkt, sizeof(*pkt) - 1);
}
