#include "board_config.h"
#include "ble_adv.h"
#include "loadcell_packet.h"
#include "loadcell_sensor.h"
#include "loadcell_types.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"

static const char *TAG = "LOADCELL_TX";

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(loadcell_sensor_init());

    loadcell_adv_pkt_t packet;
    loadcell_packet_init(&packet);
    ESP_ERROR_CHECK(ble_adv_init(&packet));

    while (1) {
        loadcell_data_t reading;
        if (loadcell_read(&reading) == ESP_OK) {
            loadcell_packet_fill_from_reading(&packet, &reading);
            ble_adv_set_packet(&packet);
            if (ble_adv_is_synced()) {
                ble_adv_start_advertising();
            }
        } else {
            ESP_LOGE(TAG, "loadcell_read failed");
        }

        vTaskDelay(pdMS_TO_TICKS(ADV_UPDATE_INTERVAL_MS));
    }
}
