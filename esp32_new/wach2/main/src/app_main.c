#include "board_config.h"
#include "sensor_node.h"
#include "ble_adv.h"
#include "vital_packet.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "nvs_flash.h"

/* =========================
 * app_main
 * ========================= */
void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(sensor_node_init());
    ESP_ERROR_CHECK(ble_init());

    xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, NULL);

    vital_adv_pkt_t adv_pkt;
    vital_packet_init(&adv_pkt);

    while (1) {
        vital_data_t vd;

        sensor_get_latest(&vd);
        packet_fill_from_sensor(&adv_pkt, &vd);
        ble_set_packet(&adv_pkt);

        if (ble_is_synced()) {
            ble_start_advertising();
        }

        vTaskDelay(pdMS_TO_TICKS(ADV_UPDATE_INTERVAL_MS));
    }
}
