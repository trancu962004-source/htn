#include "gateway_handler.h"

#include "esp_log.h"

#include "node_store.h"
#include "packet.h"
#include "uart_bridge.h"

static const char *TAG = "GW_HANDLER";

void handle_vital_packet(const vital_adv_pkt_t *pkt)
{
    if (!pkt) return;

    if (!packet_belongs(pkt->wing_id, pkt->room_id, pkt->bed_id)) {
        return;
    }

    int idx = node_store_find_or_alloc(pkt->wing_id, pkt->room_id, pkt->bed_id,
                                       pkt->patient_id, pkt->node_type);
    if (idx < 0) {
        ESP_LOGW(TAG, "Node table full");
        return;
    }

    node_state_t *node = node_store_get(idx);
    if (!node) {
        return;
    }

    if (node->has_vital && pkt->seq == node->last_seq) {
        return;
    }

    node->last_seen_ms = (uint32_t)esp_log_timestamp();
    node->last_seq = pkt->seq;
    node->has_vital = true;
    node->hr = pkt->hr;
    node->spo2 = pkt->spo2;
    node->temp_x100 = pkt->temp_x100;
    node->status = pkt->status;
    node->battery = pkt->battery;

    ESP_LOGI(TAG,
             "VITAL | W=%c R=%u B=%u P=%u | HR=%u SpO2=%u Temp=%.2f Bat=%u Seq=%u",
             pkt->wing_id,
             pkt->room_id,
             pkt->bed_id,
             pkt->patient_id,
             pkt->hr,
             pkt->spo2,
             pkt->temp_x100 / 100.0f,
             pkt->battery,
             pkt->seq);

    uart_bridge_send_vital(pkt);
}

void handle_loadcell_packet(const loadcell_adv_pkt_t *pkt)
{
    if (!pkt) return;

    if (!packet_belongs(pkt->wing_id, pkt->room_id, pkt->bed_id)) {
        return;
    }

    int idx = node_store_find_or_alloc(pkt->wing_id, pkt->room_id, pkt->bed_id,
                                       pkt->patient_id, pkt->node_type);
    if (idx < 0) {
        ESP_LOGW(TAG, "Node table full");
        return;
    }

    node_state_t *node = node_store_get(idx);
    if (!node) {
        return;
    }

    bool same_value = node->has_loadcell &&
                      node->volume_ml == pkt->volume_ml &&
                      node->drops_per_min == pkt->drops_per_min;

    if (node->has_loadcell && pkt->seq == node->last_seq) {
        return;
    }

    node->last_seen_ms = (uint32_t)esp_log_timestamp();
    node->last_seq = pkt->seq;
    node->has_loadcell = true;
    node->volume_ml = pkt->volume_ml;
    node->drops_per_min = pkt->drops_per_min;

    if (same_value) {
        return;
    }

    ESP_LOGI(TAG,
             "LOADCELL | W=%c R=%u B=%u P=%u | ML=%u DPM=%u Seq=%u",
             pkt->wing_id,
             pkt->room_id,
             pkt->bed_id,
             pkt->patient_id,
             pkt->volume_ml,
             pkt->drops_per_min,
             pkt->seq);

    uart_bridge_send_loadcell(pkt);
}
