#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "nvs_flash.h"

#include "ble_scan.h"
#include "keypad.h"
#include "lcd_i2c.h"
#include "lcd_task.h"

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    keypad_init();
    ESP_ERROR_CHECK(lcd_init_hw());
    ESP_ERROR_CHECK(ble_scan_init());

    xTaskCreate(keypad_task, "keypad_task", 3072, NULL, 5, NULL);
    xTaskCreate(lcd_task, "lcd_task", 4096, NULL, 5, NULL);
}
