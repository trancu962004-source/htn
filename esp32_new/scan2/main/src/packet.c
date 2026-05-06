#include "packet.h"

#include <string.h>
#include "board_config.h"

uint8_t calc_checksum(const uint8_t *data, uint16_t len)
{
    uint8_t sum = 0;
    for (uint16_t i = 0; i < len; i++) {
        sum ^= data[i];
    }
    return sum;
}

bool parse_vital_packet(const uint8_t *mfg, uint8_t len, vital_adv_pkt_t *pkt)
{
    if (!mfg || !pkt) return false;
    if (len < 2 + sizeof(vital_adv_pkt_t)) return false;
    if (mfg[0] != COMPANY_ID_LOW || mfg[1] != COMPANY_ID_HIGH) return false;

    memcpy(pkt, &mfg[2], sizeof(vital_adv_pkt_t));
    if (pkt->magic != PACKET_MAGIC) return false;
    if (pkt->node_type != NODE_TYPE_VITAL) return false;

    uint8_t cs = calc_checksum((const uint8_t *)pkt, sizeof(vital_adv_pkt_t) - 1);
    return cs == pkt->checksum;
}

bool parse_loadcell_packet(const uint8_t *mfg, uint8_t len, loadcell_adv_pkt_t *pkt)
{
    if (!mfg || !pkt) return false;
    if (len < 2 + sizeof(loadcell_adv_pkt_t)) return false;
    if (mfg[0] != COMPANY_ID_LOW || mfg[1] != COMPANY_ID_HIGH) return false;

    memcpy(pkt, &mfg[2], sizeof(loadcell_adv_pkt_t));
    if (pkt->magic != PACKET_MAGIC) return false;
    if (pkt->node_type != NODE_TYPE_LOADCELL) return false;

    uint8_t cs = calc_checksum((const uint8_t *)pkt, sizeof(loadcell_adv_pkt_t) - 1);
    return cs == pkt->checksum;
}
