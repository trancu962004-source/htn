#include "sensor_node.h"
#include "board_config.h"
#include "pulseox.h"
#include "max30100.h"

#include <math.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "driver/i2c.h"

static const char *TAG = "WATCH_TX_REAL";

/* =========================
 * Globals
 * ========================= */
// thông tin cảm biến MAX30100.
static max30100_t g_sensor;
// trạng thái xử lý nhịp tim/SpO2.
static pulseox_state_t g_pox;
// dữ liệu mới nhất để BLE lấy
static vital_data_t g_latest_vital = {0};
static portMUX_TYPE g_data_lock = portMUX_INITIALIZER_UNLOCKED;

/* =========================
 * Time helpers
 * ========================= */
static int64_t millis_now(void)
{
    return (int64_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

/* =========================
 * I2C init
 * ========================= */
static void i2c_master_init_simple(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA_GPIO,
        .scl_io_num = I2C_SCL_GPIO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_FREQ_HZ,
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_PORT, &conf));

    esp_err_t err = i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_ERROR_CHECK(err);
    }
}

/* =========================
 * Sensor init
 * ========================= */
esp_err_t sensor_node_init(void)
{
    i2c_master_init_simple();

    ESP_ERROR_CHECK(max30100_init(&g_sensor, I2C_PORT, MAX30100_I2C_ADDR));
    ESP_ERROR_CHECK(max30100_start_default(&g_sensor));
    ESP_ERROR_CHECK(max30100_clear_fifo(&g_sensor));

    ESP_ERROR_CHECK(
        max30100_set_spo2_config(&g_sensor,
                                 MAX30100_SR_50HZ,
                                 MAX30100_LED_PW_1600US,
                                 true)
    );

    ESP_ERROR_CHECK(
        max30100_set_led_current(&g_sensor,
                                 MAX30100_LED_CURR_11_0MA,
                                 MAX30100_LED_CURR_11_0MA)
    );

    ESP_ERROR_CHECK(max30100_set_mode(&g_sensor, MAX30100_MODE_SPO2));
    ESP_ERROR_CHECK(max30100_clear_fifo(&g_sensor));

    vTaskDelay(pdMS_TO_TICKS(200));

    ESP_ERROR_CHECK(
        max30100_set_spo2_config(&g_sensor,
                                 MAX30100_SR_100HZ,
                                 MAX30100_LED_PW_1600US,
                                 true)
    );

    ESP_ERROR_CHECK(max30100_set_mode(&g_sensor, MAX30100_MODE_SPO2));
    ESP_ERROR_CHECK(max30100_clear_fifo(&g_sensor));

    pulseox_init(&g_pox);
    pulseox_reset_report_window(&g_pox);
    pulseox_reset_spo2_window(&g_pox);

    ESP_LOGI(TAG, "MAX30100 init done");
    return ESP_OK;
}

/* =========================
 * Convert sensor state -> BLE payload data
 * ========================= */
static uint8_t calc_status(uint16_t hr, uint16_t spo2, uint16_t temp_x100)
{
    uint8_t status = 0;

    if (hr > 0 && (hr < 50 || hr > 120)) {
        status |= (1 << 0);
    }
    if (spo2 > 0 && spo2 < 94) {
        status |= (1 << 1);
    }
    if (temp_x100 > 0 && temp_x100 > 3800) {
        status |= (1 << 2);
    }

    return status;
}

/* =========================
 * Sensor task: poll MAX30100 liên tục
 * ========================= */
void sensor_task(void *arg)
{
    (void)arg;

    int64_t last_spo2_ms = millis_now();
    int64_t last_report_ms = millis_now();

    while (1) {
        max30100_sample_t samples[16];
        size_t n = 0;

        esp_err_t err = max30100_poll_samples(&g_sensor, samples, 16, &n);
        if (err == ESP_OK) {
            for (size_t i = 0; i < n; i++) {
                uint32_t ir = samples[i].ir;
                uint32_t red = samples[i].red;

                g_pox.last_ir = ir;
                g_pox.last_red = red;

                if (sample_is_saturated(ir, red)) {
                    g_pox.sat_samples++;
                    continue;
                }

                if (!sample_is_valid(ir, red)) {
                    g_pox.bad_samples++;
                    continue;
                }

                pulseox_process_sample(&g_pox, ir, red);
            }
        } else {
            ESP_LOGW(TAG, "max30100_poll_samples failed: %s", esp_err_to_name(err));
        }

        int64_t now = millis_now();

        if ((now - last_spo2_ms) >= SPO2_INTERVAL_MS) {
            float spo2 = 0.0f;
            bool ok = pulseox_compute_spo2(&g_pox, &spo2);

            if (ok) {
                if (!g_pox.spo2_valid) {
                    g_pox.spo2_value = spo2;
                    g_pox.spo2_valid = true;
                } else {
                    g_pox.spo2_value = 0.7f * g_pox.spo2_value + 0.3f * spo2;
                }
            }

            pulseox_reset_spo2_window(&g_pox);
            last_spo2_ms = now;
        }

        vital_data_t tmp = {0};
        tmp.hr = (g_pox.hr_bpm > 0.0f) ? (uint16_t)lroundf(g_pox.hr_bpm) : 0;
        tmp.spo2 = (g_pox.spo2_valid) ? (uint16_t)lroundf(g_pox.spo2_value) : 0;
        tmp.temp_x100 = 0;   
        tmp.battery = 100;
        tmp.status = calc_status(tmp.hr, tmp.spo2, tmp.temp_x100);

        portENTER_CRITICAL(&g_data_lock);
        g_latest_vital = tmp;
        portEXIT_CRITICAL(&g_data_lock);

        if ((now - last_report_ms) >= REPORT_INTERVAL_MS) {
            ESP_LOGI(TAG,
                     "REAL DATA | HR=%u bpm | SpO2=%u %% | Temp=%.2f | good=%lu bad=%lu sat=%lu | IR=%lu RED=%lu",
                     tmp.hr,
                     tmp.spo2,
                     tmp.temp_x100 / 100.0f,
                     (unsigned long)g_pox.good_samples,
                     (unsigned long)g_pox.bad_samples,
                     (unsigned long)g_pox.sat_samples,
                     (unsigned long)g_pox.last_ir,
                     (unsigned long)g_pox.last_red);

            pulseox_reset_report_window(&g_pox);
            last_report_ms = now;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void sensor_get_latest(vital_data_t *vd)
{
    portENTER_CRITICAL(&g_data_lock);
    *vd = g_latest_vital;
    portEXIT_CRITICAL(&g_data_lock);
}
