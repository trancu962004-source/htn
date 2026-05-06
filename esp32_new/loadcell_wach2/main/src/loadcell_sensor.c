#include "loadcell_sensor.h"
#include "board_config.h"
#include "drop_sensor.h"
#include "hx711.h"

#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "LOADCELL_SENSOR";

esp_err_t loadcell_sensor_init(void)
{
    ESP_RETURN_ON_ERROR(hx711_gpio_init(), TAG, "hx711 init failed");
    ESP_RETURN_ON_ERROR(drop_sensor_init(), TAG, "drop sensor init failed");
    vTaskDelay(pdMS_TO_TICKS(HX711_STARTUP_DELAY_MS));
    ESP_RETURN_ON_ERROR(hx711_tare(HX711_SAMPLES), TAG, "hx711 tare failed");
    return ESP_OK;
}

esp_err_t loadcell_read(loadcell_data_t *out)
{
    if (out == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    int32_t raw = 0;
    ESP_RETURN_ON_ERROR(hx711_read_average(HX711_SAMPLES, &raw), TAG, "read average failed");

    float weight_g = ((float)(raw - hx711_get_offset())) / LOADCELL_SCALE_FACTOR;
    float volume_ml = weight_g;

    if (volume_ml < 0.0f && volume_ml > NEGATIVE_CLAMP_ML) {
        volume_ml = 0.0f;
    }
    if (volume_ml < 0.0f) {
        volume_ml = 0.0f;
    }
    if (volume_ml > 65535.0f) {
        volume_ml = 65535.0f;
    }

    out->volume_ml = (uint16_t)(volume_ml + 0.5f);
    out->drops_per_min = drop_sensor_calc_drops_per_min();

    ESP_LOGI(TAG,
             "HX711 | raw=%" PRId32 " ml=%u dpm=%u",
             raw, out->volume_ml, out->drops_per_min);

    return ESP_OK;
}
