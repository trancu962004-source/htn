#ifndef APP_RTOS_H
#define APP_RTOS_H

#include <stdint.h>
#include "app_config.h"

/* Message placed into the Queue by UART Interrupt, then consumed by Gateway Task. */
typedef struct
{
    uint8_t port;                    /* 2 = UART2, 3 = UART3 */
    char line[UART_LINE_MAX_LEN];    /* Full UART line without CR/LF */
} UartLineMessage_t;

void RTOS_InitObjects(void);
void RTOS_StartScheduler(void);
uint8_t RTOS_IsRunning(void);

#if APP_USE_FREERTOS

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

extern QueueHandle_t uartLineQueueHandle;
extern SemaphoreHandle_t debugUartMutexHandle;
extern SemaphoreHandle_t canTxMutexHandle;
extern SemaphoreHandle_t canErrorSemaphoreHandle;

void Gateway_Task(void *argument);
void CAN_Error_Task(void *argument);
void Test_Task(void *argument);
void Heartbeat_Timer_Callback(TimerHandle_t xTimer);

BaseType_t RTOS_QueueUartLineFromISR(uint8_t port,
                                     const char *line,
                                     BaseType_t *pxHigherPriorityTaskWoken);
void RTOS_GiveCanErrorSemaphoreFromISR(BaseType_t *pxHigherPriorityTaskWoken);

uint8_t RTOS_LockDebugUart(void);
void RTOS_UnlockDebugUart(void);
uint8_t RTOS_LockCanTx(void);
void RTOS_UnlockCanTx(void);

#endif /* APP_USE_FREERTOS */

#endif /* APP_RTOS_H */
