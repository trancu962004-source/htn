#pragma once

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t lcd_init_hw(void);
void lcd_print_line(uint8_t row, const char *text);

#ifdef __cplusplus
}
#endif
