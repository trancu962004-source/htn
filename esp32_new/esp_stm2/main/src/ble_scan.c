#include "ble_scan.h"

#include <string.h>

#include "esp_bt.h"
#include "esp_err.h"
#include "esp_log.h"

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"

#include "board_config.h"
#include "gateway_handler.h"
#include "packet.h"

static const char *TAG = "BLE_SCAN";
static uint8_t own_addr_type = 0;

static void start_scan(void);

static int gap_event_cb(struct ble_gap_event *event, void *arg)
{
    (void)arg;

    switch (event->type) {
    case BLE_GAP_EVENT_DISC:
    {
        struct ble_hs_adv_fields fields;
        memset(&fields, 0, sizeof(fields));

        int rc = ble_hs_adv_parse_fields(&fields, event->disc.data, event->disc.length_data);
        if (rc != 0 || fields.mfg_data == NULL) {
            return 0;
        }

        const uint8_t *mfg = fields.mfg_data;
        uint8_t mfg_len = fields.mfg_data_len;
        if (mfg_len < 2 + 6) {
            return 0;
        }

        uint8_t node_type = mfg[2 + 5];

        if (node_type == NODE_TYPE_VITAL) {
            vital_adv_pkt_t pkt;
            if (parse_vital_packet(mfg, mfg_len, &pkt)) {
                handle_vital_packet(&pkt);
            }
        } else if (node_type == NODE_TYPE_LOADCELL) {
            loadcell_adv_pkt_t pkt;
            if (parse_loadcell_packet(mfg, mfg_len, &pkt)) {
                handle_loadcell_packet(&pkt);
            }
        }
        return 0;
    }

    case BLE_GAP_EVENT_DISC_COMPLETE:
        ESP_LOGW(TAG, "Scan complete -> restart");
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

    int rc = ble_gap_disc(own_addr_type, BLE_HS_FOREVER, &disc_params, gap_event_cb, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gap_disc rc=%d", rc);
    } else {
        ESP_LOGI(TAG, "Scan start | room=%d wing=%c beds=%d..%d",
                 MY_ROOM_ID, MY_WING_ID, MANAGED_BED_START, MANAGED_BED_END);
    }
}

static void on_sync(void)
{
    int rc = ble_hs_id_infer_auto(0, &own_addr_type);
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

    int rc = nimble_port_init();
    if (rc != 0) {
        ESP_LOGE(TAG, "nimble_port_init rc=%d", rc);
        return ESP_FAIL;
    }

    ble_svc_gap_init();
    ble_hs_cfg.sync_cb = on_sync;
    nimble_port_freertos_init(ble_host_task);

    return ESP_OK;
}
