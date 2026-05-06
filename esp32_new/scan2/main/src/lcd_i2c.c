#include "lcd_i2c.h"

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_rom_sys.h"

#include "board_config.h"

static esp_err_t lcd_write_raw(uint8_t data)
{
    return i2c_master_write_to_device(I2C_PORT, LCD_I2C_ADDR, &data, 1, pdMS_TO_TICKS(50));
}

static void lcd_pulse(uint8_t data)
{
    lcd_write_raw(data | LCD_ENABLE | LCD_BACKLIGHT);
    esp_rom_delay_us(1);
    lcd_write_raw((data & ~LCD_ENABLE) | LCD_BACKLIGHT);
    esp_rom_delay_us(50);
}

static void lcd_write4(uint8_t nibble, uint8_t mode)
{
    uint8_t data = (nibble & 0xF0) | mode | LCD_BACKLIGHT;
    lcd_write_raw(data);
    lcd_pulse(data);
}

static void lcd_send(uint8_t value, uint8_t mode)
{
    lcd_write4(value & 0xF0, mode);
    lcd_write4((value << 4) & 0xF0, mode);
}

static void lcd_cmd(uint8_t cmd)
{
    lcd_send(cmd, 0);
    if (cmd == 0x01 || cmd == 0x02) {
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

static void lcd_data(uint8_t data)
{
    lcd_send(data, LCD_RS);
}

static void lcd_set_cursor(uint8_t col, uint8_t row)
{
    static const uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    if (row > 3) row = 3;
    lcd_cmd(0x80 | (col + row_offsets[row]));
}

void lcd_print_line(uint8_t row, const char *text)
{
    char buf[21];
    size_t n = strlen(text);
    if (n > 20) n = 20;

    memset(buf, ' ', 20);
    memcpy(buf, text, n);
    buf[20] = '\0';

    lcd_set_cursor(0, row);
    for (int i = 0; i < 20; i++) {
        lcd_data((uint8_t)buf[i]);
    }
}

esp_err_t lcd_init_hw(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA_GPIO,
        .scl_io_num = I2C_SCL_GPIO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_FREQ_HZ,
        .clk_flags = 0,
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_PORT, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0));

    vTaskDelay(pdMS_TO_TICKS(50));
    lcd_write4(0x30, 0);
    vTaskDelay(pdMS_TO_TICKS(5));
    lcd_write4(0x30, 0);
    esp_rom_delay_us(150);
    lcd_write4(0x30, 0);
    lcd_write4(0x20, 0);

    lcd_cmd(0x28);
    lcd_cmd(0x0C);
    lcd_cmd(0x06);
    lcd_cmd(0x01);
    return ESP_OK;
}
