#include "pulseox.h"
#include "board_config.h"

#include <math.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* =========================
 * Time helpers
 * ========================= */
static int64_t millis_now(void)
{
    return (int64_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

/* =========================
 * MAX30100 signal processing
 * ========================= */
void pulseox_init(pulseox_state_t *st)
{
    memset(st, 0, sizeof(*st));
    st->peak_armed = true;
    st->last_peak_ms = 0;
    st->hr_bpm = 0.0f;
    st->spo2_value = 0.0f;
    st->spo2_valid = false;
}

void pulseox_reset_report_window(pulseox_state_t *st)
{
    st->good_samples = 0;
    st->bad_samples = 0;
    st->sat_samples = 0;
}

void pulseox_reset_spo2_window(pulseox_state_t *st)
{
    st->spo2_ir_ac_sq_sum = 0.0f;
    st->spo2_red_ac_sq_sum = 0.0f;
    st->spo2_ir_dc_sum = 0.0f;
    st->spo2_red_dc_sum = 0.0f;
    st->spo2_sample_count = 0;
}

bool sample_is_saturated(uint32_t ir, uint32_t red)
{
    return (ir >= SATURATION_LIMIT || red >= SATURATION_LIMIT);
}

static bool sample_is_weird_pattern(uint32_t ir, uint32_t red)
{
    if ((ir == 8192U  && red == 16384U) ||
        (ir == 16384U && red == 8192U)  ||
        (ir == 16384U && red == 32768U) ||
        (ir == 32768U && red == 16384U)) {
        return true;
    }
    return false;
}

bool sample_is_valid(uint32_t ir, uint32_t red)
{
    if (ir == 0 || red == 0) {
        return false;
    }

    if (sample_is_saturated(ir, red)) {
        return false;
    }

    if (ir < MIN_VALID_IR || red < MIN_VALID_RED) {
        return false;
    }

    if (sample_is_weird_pattern(ir, red)) {
        return false;
    }

    return true;
}

void pulseox_process_sample(pulseox_state_t *st, uint32_t ir, uint32_t red)
{
    st->last_ir = ir;
    st->last_red = red;

    const float alpha = 0.95f;

    if (st->ir_dc == 0.0f) {
        st->ir_dc = (float)ir;
    }
    if (st->red_dc == 0.0f) {
        st->red_dc = (float)red;
    }

    st->ir_dc  = alpha * st->ir_dc  + (1.0f - alpha) * (float)ir;
    st->red_dc = alpha * st->red_dc + (1.0f - alpha) * (float)red;

    float ir_ac  = (float)ir  - st->ir_dc;
    float red_ac = (float)red - st->red_dc;

    st->good_samples++;

    st->spo2_ir_ac_sq_sum  += ir_ac * ir_ac;
    st->spo2_red_ac_sq_sum += red_ac * red_ac;
    st->spo2_ir_dc_sum     += st->ir_dc;
    st->spo2_red_dc_sum    += st->red_dc;
    st->spo2_sample_count++;

    {
        float prev = st->ir_prev;
        float curr = st->ir_curr;
        float next = ir_ac;

        float threshold = 120.0f;

        if (curr > threshold && curr > prev && curr > next) {
            int64_t now = millis_now();

            if (st->peak_armed) {
                if (st->last_peak_ms != 0) {
                    float dt_ms = (float)(now - st->last_peak_ms);

                    if (dt_ms >= 333.0f && dt_ms <= 1333.0f) {
                        float bpm = 60000.0f / dt_ms;

                        if (bpm >= HR_MIN_BPM && bpm <= HR_MAX_BPM) {
                            if (st->hr_bpm == 0.0f) {
                                st->hr_bpm = bpm;
                            } else {
                                st->hr_bpm = 0.75f * st->hr_bpm + 0.25f * bpm;
                            }
                        }
                    }
                }

                st->last_peak_ms = now;
                st->peak_armed = false;
            }
        }

        if (curr < 30.0f) {
            st->peak_armed = true;
        }

        st->ir_prev = st->ir_curr;
        st->ir_curr = ir_ac;
    }
}

bool pulseox_compute_spo2(pulseox_state_t *st, float *spo2_out)
{
    if (st->spo2_sample_count < 400) {
        return false;
    }

    float ir_dc_avg  = st->spo2_ir_dc_sum  / (float)st->spo2_sample_count;
    float red_dc_avg = st->spo2_red_dc_sum / (float)st->spo2_sample_count;

    if (ir_dc_avg < 100.0f || red_dc_avg < 100.0f) {
        return false;
    }

    float ir_ac_rms  = sqrtf(st->spo2_ir_ac_sq_sum  / (float)st->spo2_sample_count);
    float red_ac_rms = sqrtf(st->spo2_red_ac_sq_sum / (float)st->spo2_sample_count);

    if (ir_ac_rms < 20.0f || red_ac_rms < 20.0f) {
        return false;
    }

    float ir_norm  = ir_ac_rms / ir_dc_avg;
    float red_norm = red_ac_rms / red_dc_avg;

    if (ir_norm <= 0.0f || red_norm <= 0.0f) {
        return false;
    }

    float r = red_norm / ir_norm;

    if (r < 0.4f || r > 1.3f) {
        return false;
    }

    float spo2 = 104.0f - 17.0f * r;

    if (spo2 > 100.0f) spo2 = 100.0f;
    if (spo2 < 85.0f)  spo2 = 85.0f;

    *spo2_out = spo2;
    return true;
}
