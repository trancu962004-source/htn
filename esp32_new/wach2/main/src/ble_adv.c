#include "ble_adv.h"
#include "board_config.h"

#include <string.h>

#include "esp_bt.h"
#include "esp_log.h"

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"

static const char *TAG = "WATCH_TX_REAL";

static uint8_t s_own_addr_type = 0;
static bool s_ble_synced = false;

static vital_adv_pkt_t s_adv_pkt = {
    .magic = PACKET_MAGIC,
    .wing_id = (uint8_t)WING_ID,
    .room_id = ROOM_ID,
    .bed_id = BED_ID,
    .patient_id = PATIENT_ID,
    .node_type = NODE_TYPE_VITAL,
};

bool ble_is_synced(void)
{
    return s_ble_synced;
}

void ble_set_packet(const vital_adv_pkt_t *pkt)
{
    s_adv_pkt = *pkt;
}

static int ble_gap_event_cb(struct ble_gap_event *event, void *arg)
{
    (void)arg;

    if (event->type == BLE_GAP_EVENT_ADV_COMPLETE) {
        ESP_LOGI(TAG, "Advertising complete, restarting");
    }
    return 0;
}

void ble_start_advertising(void)
{
    if (!s_ble_synced) {
        return;
    }

    int rc = ble_gap_adv_stop();
    if (rc != 0 && rc != BLE_HS_EALREADY) {
        ESP_LOGW(TAG, "ble_gap_adv_stop rc=%d", rc);
    }

    uint8_t mfg_data[2 + sizeof(vital_adv_pkt_t)] = {0};
    mfg_data[0] = BLE_COMPANY_ID_LOW;
    mfg_data[1] = BLE_COMPANY_ID_HIGH;
    memcpy(&mfg_data[2], &s_adv_pkt, sizeof(s_adv_pkt));

    struct ble_hs_adv_fields adv_fields;
    memset(&adv_fields, 0, sizeof(adv_fields));
    adv_fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    adv_fields.mfg_data = mfg_data;
    adv_fields.mfg_data_len = sizeof(mfg_data);

    rc = ble_gap_adv_set_fields(&adv_fields);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gap_adv_set_fields rc=%d", rc);
        return;
    }

    struct ble_hs_adv_fields rsp_fields;
    memset(&rsp_fields, 0, sizeof(rsp_fields));
    const char *name = ble_svc_gap_device_name();
    rsp_fields.name = (const uint8_t *)name;
    rsp_fields.name_len = strlen(name);
    rsp_fields.name_is_complete = 1;

    rc = ble_gap_adv_rsp_set_fields(&rsp_fields);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gap_adv_rsp_set_fields rc=%d", rc);
        return;
    }

    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_NON;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    rc = ble_gap_adv_start(s_own_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, ble_gap_event_cb, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gap_adv_start rc=%d", rc);
        return;
    }

    ESP_LOGI(TAG,
             "ADV REAL | W=%c R=%u B=%u P=%u | HR=%u SpO2=%u Temp=%.2f Bat=%u Status=0x%02X Seq=%u",
             s_adv_pkt.wing_id,
             s_adv_pkt.room_id,
             s_adv_pkt.bed_id,
             s_adv_pkt.patient_id,
             s_adv_pkt.hr,
             s_adv_pkt.spo2,
             s_adv_pkt.temp_x100 / 100.0f,
             s_adv_pkt.battery,
             s_adv_pkt.status,
             s_adv_pkt.seq);
}

static void ble_on_sync(void)
{
    int rc = ble_hs_id_infer_auto(0, &s_own_addr_type);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_hs_id_infer_auto rc=%d", rc);
        return;
    }

    s_ble_synced = true;
    ESP_LOGI(TAG, "BLE synced");
    ble_start_advertising();
}

static void ble_host_task(void *param)
{
    (void)param;
    nimble_port_run();
    nimble_port_freertos_deinit();
}

esp_err_t ble_init(void)
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
    rc = ble_svc_gap_device_name_set(DEVICE_NAME);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_svc_gap_device_name_set rc=%d", rc);
        return ESP_FAIL;
    }

    ble_hs_cfg.sync_cb = ble_on_sync;
    nimble_port_freertos_init(ble_host_task);
    return ESP_OK;
}
