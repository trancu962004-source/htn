#include "lcd_task.h"

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app_state.h"
#include "board_config.h"
#include "lcd_i2c.h"

void lcd_task(void *arg)
{
    (void)arg;
    char line1[21];
    char line2[21];
    char line3[21];
    char line4[21];

    while (1) {
        uint32_t now = now_ms();
        bool watch_ok = s_sel.has_watch && ((now - s_sel.watch_seen_ms) < DATA_TIMEOUT_MS);
        bool load_ok  = s_sel.has_loadcell && ((now - s_sel.load_seen_ms) < DATA_TIMEOUT_MS);

        if (s_input_mode == INPUT_NONE) {
            char wing_buf[4];
            char room_buf[4];
            char bed_buf[4];
            char patient_buf[4];

            if (s_sel_wing >= 'A' && s_sel_wing <= 'Z') {
                snprintf(wing_buf, sizeof(wing_buf), "%c", s_sel_wing);
            } else {
                snprintf(wing_buf, sizeof(wing_buf), "-");
            }

            if (s_sel_room > 0) {
                snprintf(room_buf, sizeof(room_buf), "%02u", (unsigned)s_sel_room);
            } else {
                snprintf(room_buf, sizeof(room_buf), "--");
            }

            if (s_sel_bed > 0) {
                snprintf(bed_buf, sizeof(bed_buf), "%02u", (unsigned)s_sel_bed);
            } else {
                snprintf(bed_buf, sizeof(bed_buf), "--");
            }

            if (s_sel_patient > 0) {
                snprintf(patient_buf, sizeof(patient_buf), "%02u", (unsigned)s_sel_patient);
            } else {
                snprintf(patient_buf, sizeof(patient_buf), "--");
            }

            snprintf(line1, sizeof(line1),
                     "W%s R%s B%s P%s",
                     wing_buf, room_buf, bed_buf, patient_buf);
        } else {
            char mode_char = '?';
            if (s_input_mode == INPUT_W) mode_char = 'W';
            if (s_input_mode == INPUT_R) mode_char = 'R';
            if (s_input_mode == INPUT_B) mode_char = 'B';
            if (s_input_mode == INPUT_P) mode_char = 'P';

            snprintf(line1, sizeof(line1), "SET %c:%s", mode_char, s_input_buf);
        }

        if (load_ok) {
            snprintf(line2, sizeof(line2),
                     "ML:%u DPM:%u",
                     (unsigned)s_sel.ml,
                     (unsigned)s_sel.dpm);
        } else {
            snprintf(line2, sizeof(line2), "ML:--- DPM:---");
        }

        if (watch_ok) {
            snprintf(line3, sizeof(line3),
                     "HR:%u SpO2:%u",
                     (unsigned)s_sel.hr,
                     (unsigned)s_sel.spo2);

            snprintf(line4, sizeof(line4),
                     "Temp:%u.%02uC",
                     (unsigned)(s_sel.temp_x100 / 100),
                     (unsigned)(s_sel.temp_x100 % 100));
        } else {
            snprintf(line3, sizeof(line3), "HR:--- SpO2:---");
            snprintf(line4, sizeof(line4), "Temp:---.--C");
        }

        lcd_print_line(0, line1);
        lcd_print_line(1, line2);
        lcd_print_line(2, line3);
        lcd_print_line(3, line4);

        vTaskDelay(pdMS_TO_TICKS(LCD_REFRESH_MS));
    }
}
