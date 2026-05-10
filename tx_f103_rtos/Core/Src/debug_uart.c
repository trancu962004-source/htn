#include "debug_uart.h"
#include "app_config.h"
#include "app_handles.h"
#include "app_rtos.h"

#include <string.h>

void UART1_Print(const char *msg)
{
    uint8_t locked = 0U;

    if (msg == NULL)
    {
        return;
    }

#if APP_USE_FREERTOS
    locked = RTOS_LockDebugUart();
#endif

    HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);

#if APP_USE_FREERTOS
    if (locked)
    {
        RTOS_UnlockDebugUart();
    }
#else
    (void)locked;
#endif
}

void UART1_EchoByte(uint8_t port, uint8_t ch)
{
#if ENABLE_UART_BYTE_ECHO
    static uint8_t u2_new_line = 1;
    static uint8_t u3_new_line = 1;

    if (port == 2U)
    {
        if (u2_new_line)
        {
            UART1_Print("\r\n[UART2 RX] ");
            u2_new_line = 0;
        }

        HAL_UART_Transmit(&huart1, &ch, 1, 10);

        if (ch == '\n')
        {
            u2_new_line = 1;
        }
    }
    else if (port == 3U)
    {
        if (u3_new_line)
        {
            UART1_Print("\r\n[UART3 RX] ");
            u3_new_line = 0;
        }

        HAL_UART_Transmit(&huart1, &ch, 1, 10);

        if (ch == '\n')
        {
            u3_new_line = 1;
        }
    }
#else
    (void)port;
    (void)ch;
#endif
}
