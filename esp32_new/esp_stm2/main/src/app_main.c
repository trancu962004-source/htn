#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "ble_scan.h"
#include "node_store.h"
#include "uart_bridge.h"

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    node_store_init();
    uart_bridge_init();
    ESP_ERROR_CHECK(ble_scan_init());

    xTaskCreate(node_monitor_task, "node_monitor_task", 4096, NULL, 5, NULL);
}
