#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool     has_watch;
    bool     has_loadcell;

    uint16_t hr;
    uint16_t spo2;
    uint16_t temp_x100;

    uint16_t ml;
    uint16_t dpm;

    uint8_t  watch_seq;
    uint8_t  load_seq;

    uint32_t watch_seen_ms;
    uint32_t load_seen_ms;
} selected_data_t;

typedef enum {
    INPUT_NONE = 0,
    INPUT_W,
    INPUT_R,
    INPUT_B,
    INPUT_P
} input_mode_t;

extern volatile char s_sel_wing;
extern volatile uint8_t s_sel_room;
extern volatile uint8_t s_sel_bed;
extern volatile uint8_t s_sel_patient;

extern volatile input_mode_t s_input_mode;
extern char s_input_buf[8];
extern uint8_t s_input_len;

extern selected_data_t s_sel;

uint32_t now_ms(void);

#ifdef __cplusplus
}
#endif
