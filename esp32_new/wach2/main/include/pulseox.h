#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    //thành phần nền DC của tín hiệu.
    float ir_dc;
    float red_dc;
    // dùng phát hiện đỉnh sóng.
    float ir_prev;
    float ir_curr;
    //cho phép bắt đỉnh tiếp theo.
    bool peak_armed;
    int64_t last_peak_ms;
    //nhịp tim.
    float hr_bpm;

    uint32_t last_ir;
    uint32_t last_red;

    uint32_t good_samples;
    uint32_t bad_samples;
    uint32_t sat_samples;

    float spo2_ir_ac_sq_sum;
    float spo2_red_ac_sq_sum;
    float spo2_ir_dc_sum;
    float spo2_red_dc_sum;
    uint32_t spo2_sample_count;

    float spo2_value;
    bool spo2_valid;
} pulseox_state_t;

void pulseox_init(pulseox_state_t *st);
void pulseox_reset_report_window(pulseox_state_t *st);
void pulseox_reset_spo2_window(pulseox_state_t *st);
bool sample_is_saturated(uint32_t ir, uint32_t red);
bool sample_is_valid(uint32_t ir, uint32_t red);
void pulseox_process_sample(pulseox_state_t *st, uint32_t ir, uint32_t red);
bool pulseox_compute_spo2(pulseox_state_t *st, float *spo2_out);
