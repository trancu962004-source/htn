#include "uart_line.h"
#include "app_config.h"
#include "app_handles.h"
#include "debug_uart.h"
#include "app_rtos.h"

#include <string.h>

static uint8_t uart2_rx_byte;
static char uart2_rx_line[UART_LINE_MAX_LEN];
char uart2_line_copy[UART_LINE_MAX_LEN];
static volatile uint16_t uart2_rx_index = 0;
volatile uint8_t uart2_line_ready = 0;

static uint8_t uart3_rx_byte;
static char uart3_rx_line[UART_LINE_MAX_LEN];
char uart3_line_copy[UART_LINE_MAX_LEN];
static volatile uint16_t uart3_rx_index = 0;
volatile uint8_t uart3_line_ready = 0;

void UART2_StartReceiveIT(void)
{
    if (HAL_UART_Receive_IT(&huart2, &uart2_rx_byte, 1) != HAL_OK)
    {
        UART1_Print("UART2 RX IT start failed\r\n");
    }
}

void UART3_StartReceiveIT(void)
{
    if (HAL_UART_Receive_IT(&huart3, &uart3_rx_byte, 1) != HAL_OK)
    {
        UART1_Print("UART3 RX IT start failed\r\n");
    }
}

static void UART_LineCompletedFromInterrupt(uint8_t port, const char *line)
{
#if APP_USE_FREERTOS
    if (RTOS_IsRunning())
    {
        BaseType_t higher_priority_task_woken = pdFALSE;
        (void)RTOS_QueueUartLineFromISR(port, line, &higher_priority_task_woken);
        portYIELD_FROM_ISR(higher_priority_task_woken);
        return;
    }
#endif

    if (port == 2U)
    {
        strncpy(uart2_line_copy, line, sizeof(uart2_line_copy) - 1U);
        uart2_line_copy[sizeof(uart2_line_copy) - 1U] = '\0';
        uart2_line_ready = 1U;
    }
    else if (port == 3U)
    {
        strncpy(uart3_line_copy, line, sizeof(uart3_line_copy) - 1U);
        uart3_line_copy[sizeof(uart3_line_copy) - 1U] = '\0';
        uart3_line_ready = 1U;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        UART1_EchoByte(2, uart2_rx_byte);

        if (uart2_rx_byte == '\n')
        {
            uart2_rx_line[uart2_rx_index] = '\0';
            UART_LineCompletedFromInterrupt(2U, uart2_rx_line);
            uart2_rx_index = 0;
        }
        else if (uart2_rx_byte != '\r')
        {
            if (uart2_rx_index < (sizeof(uart2_rx_line) - 1U))
            {
                uart2_rx_line[uart2_rx_index++] = (char)uart2_rx_byte;
            }
            else
            {
                uart2_rx_index = 0;
#if !APP_USE_FREERTOS
                UART1_Print("\r\nUART2 line overflow -> reset\r\n");
#endif
            }
        }

        UART2_StartReceiveIT();
    }
    else if (huart->Instance == USART3)
    {
        UART1_EchoByte(3, uart3_rx_byte);

        if (uart3_rx_byte == '\n')
        {
            uart3_rx_line[uart3_rx_index] = '\0';
            UART_LineCompletedFromInterrupt(3U, uart3_rx_line);
            uart3_rx_index = 0;
        }
        else if (uart3_rx_byte != '\r')
        {
            if (uart3_rx_index < (sizeof(uart3_rx_line) - 1U))
            {
                uart3_rx_line[uart3_rx_index++] = (char)uart3_rx_byte;
            }
            else
            {
                uart3_rx_index = 0;
#if !APP_USE_FREERTOS
                UART1_Print("\r\nUART3 line overflow -> reset\r\n");
#endif
            }
        }

        UART3_StartReceiveIT();
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
#if !APP_USE_FREERTOS
        UART1_Print("UART2 ERROR -> restart RX\r\n");
#endif
        UART2_StartReceiveIT();
    }
    else if (huart->Instance == USART3)
    {
#if !APP_USE_FREERTOS
        UART1_Print("UART3 ERROR -> restart RX\r\n");
#endif
        UART3_StartReceiveIT();
    }
}
