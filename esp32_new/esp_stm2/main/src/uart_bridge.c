#include "uart_bridge.h"

#include <stdio.h>

#include "driver/uart.h"
#include "esp_err.h"

#include "board_config.h"

void uart_bridge_init(void)
{
    const uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, UART_BUF_SIZE, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
}

void uart_bridge_send_vital(const vital_adv_pkt_t *pkt)
{
    if (!pkt) return;

    char line[160];
    int n = snprintf(line, sizeof(line),
                     "TYPE=VITAL,W=%c,R=%u,B=%u,P=%u,HR=%u,SPO2=%u,T=%.2f,BAT=%u,SEQ=%u\r\n",
                     pkt->wing_id,
                     pkt->room_id,
                     pkt->bed_id,
                     pkt->patient_id,
                     pkt->hr,
                     pkt->spo2,
                     pkt->temp_x100 / 100.0f,
                     pkt->battery,
                     pkt->seq);

    if (n > 0) {
        uart_write_bytes(UART_PORT_NUM, line, n);
    }
}

void uart_bridge_send_loadcell(const loadcell_adv_pkt_t *pkt)
{
    if (!pkt) return;

    char line[160];
    int n = snprintf(line, sizeof(line),
                     "TYPE=LOADCELL,W=%c,R=%u,B=%u,P=%u,ML=%u,DPM=%u,SEQ=%u\r\n",
                     pkt->wing_id,
                     pkt->room_id,
                     pkt->bed_id,
                     pkt->patient_id,
                     pkt->volume_ml,
                     pkt->drops_per_min,
                     pkt->seq);
    if (n > 0) {
        uart_write_bytes(UART_PORT_NUM, line, n);
    }
}
