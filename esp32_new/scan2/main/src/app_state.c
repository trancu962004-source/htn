#include "app_state.h"

#include "esp_log.h"

volatile char s_sel_wing = 0;
volatile uint8_t s_sel_room = 0;
volatile uint8_t s_sel_bed = 0;
volatile uint8_t s_sel_patient = 0;

volatile input_mode_t s_input_mode = INPUT_NONE;
char s_input_buf[8] = {0};
uint8_t s_input_len = 0;

selected_data_t s_sel = {0};

uint32_t now_ms(void)
{
    return (uint32_t)esp_log_timestamp();
}
