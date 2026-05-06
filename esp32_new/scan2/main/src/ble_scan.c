#include "ble_scan.h"

#include <string.h>

#include "esp_bt.h"
#include "esp_err.h"
#include "esp_log.h"

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"

#include "app_state.h"
#include "board_config.h"
#include "packet.h"

static const char *TAG = APP_TAG;
static uint8_t s_own_addr_type = 0;

static void handle_vital(const vital_adv_pkt_t *pkt)
{
    if (pkt->wing_id != (uint8_t)s_sel_wing ||
        pkt->room_id != s_sel_room ||
        pkt->bed_id != s_sel_bed ||
        pkt->patient_id != s_sel_patient) {
        return;
    }

    if (s_sel.has_watch && pkt->seq == s_sel.watch_seq) {
        return;
    }

    s_sel.has_watch = true;
    s_sel.hr = pkt->hr;
    s_sel.spo2 = pkt->spo2;
    s_sel.temp_x100 = pkt->temp_x100;
    s_sel.watch_seq = pkt->seq;
    s_sel.watch_seen_ms = now_ms();
}

static void handle_loadcell(const loadcell_adv_pkt_t *pkt)
{
    if (pkt->wing_id != (uint8_t)s_sel_wing ||
        pkt->room_id != s_sel_room ||
        pkt->bed_id != s_sel_bed ||
        pkt->patient_id != s_sel_patient) {
        return;
    }

    if (s_sel.has_loadcell && pkt->seq == s_sel.load_seq) {
        return;
    }

    s_sel.has_loadcell = true;
    s_sel.ml = pkt->volume_ml;
    s_sel.dpm = pkt->drops_per_min;
    s_sel.load_seq = pkt->seq;
    s_sel.load_seen_ms = now_ms();
}

static void start_scan(void);

static int gap_event_cb(struct ble_gap_event *event, void *arg)
{
    (void)arg;

    switch (event->type) {
        case BLE_GAP_EVENT_DISC: {
            struct ble_hs_adv_fields fields;
            memset(&fields, 0, sizeof(fields));

            int rc = ble_hs_adv_parse_fields(&fields, event->disc.data, event->disc.length_data);
            if (rc != 0 || fields.mfg_data == NULL) return 0;

            const uint8_t *mfg = fields.mfg_data;
            uint8_t mfg_len = fields.mfg_data_len;
            if (mfg_len < 2 + 7) return 0;

            uint8_t node_type = mfg[2 + 5];

            if (node_type == NODE_TYPE_VITAL) {
                vital_adv_pkt_t pkt;
                if (parse_vital_packet(mfg, mfg_len, &pkt)) {
                    handle_vital(&pkt);
                }
            } else if (node_type == NODE_TYPE_LOADCELL) {
                loadcell_adv_pkt_t pkt;
                if (parse_loadcell_packet(mfg, mfg_len, &pkt)) {
                    handle_loadcell(&pkt);
                }
            }
            return 0;
        }

        case BLE_GAP_EVENT_DISC_COMPLETE:
            start_scan();
            return 0;

        default:
            return 0;
    }
}

static void start_scan(void)
{
    struct ble_gap_disc_params disc_params;
    memset(&disc_params, 0, sizeof(disc_params));

    disc_params.passive = 0;
    disc_params.itvl = 0x0010;
    disc_params.window = 0x0010;
    disc_params.filter_duplicates = 0;

    int rc = ble_gap_disc(s_own_addr_type, BLE_HS_FOREVER, &disc_params, gap_event_cb, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gap_disc rc=%d", rc);
    }
}

static void on_sync(void)
{
    int rc = ble_hs_id_infer_auto(0, &s_own_addr_type);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_hs_id_infer_auto rc=%d", rc);
        return;
    }

    ESP_LOGI(TAG, "BLE synced");
    start_scan();
}

static void ble_host_task(void *param)
{
    (void)param;
    nimble_port_run();
    nimble_port_freertos_deinit();
}

esp_err_t ble_scan_init(void)
{
    esp_err_t rel = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    if (rel != ESP_OK && rel != ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "esp_bt_controller_mem_release rc=%s", esp_err_to_name(rel));
    }

    ESP_ERROR_CHECK(nimble_port_init());
    ble_svc_gap_init();
    ble_hs_cfg.sync_cb = on_sync;
    nimble_port_freertos_init(ble_host_task);

    return ESP_OK;
}
