#include "app_rtos.h"

#if APP_USE_FREERTOS

#include "can_gateway.h"
#include "debug_uart.h"
#include "gateway_process.h"
#include "main.h"

#include <string.h>

QueueHandle_t uartLineQueueHandle = NULL;
SemaphoreHandle_t debugUartMutexHandle = NULL;
SemaphoreHandle_t canTxMutexHandle = NULL;
SemaphoreHandle_t canErrorSemaphoreHandle = NULL;

static TimerHandle_t heartbeatTimerHandle = NULL;
static volatile uint8_t rtosSchedulerStarted = 0U;

uint8_t RTOS_IsRunning(void)
{
    return rtosSchedulerStarted;
}

uint8_t RTOS_LockDebugUart(void)
{
    if (__get_IPSR() != 0U)
    {
        return 0U;
    }

    if (RTOS_IsRunning() && debugUartMutexHandle != NULL)
    {
        return (xSemaphoreTake(debugUartMutexHandle, portMAX_DELAY) == pdPASS) ? 1U : 0U;
    }

    return 0U;
}

void RTOS_UnlockDebugUart(void)
{
    if (__get_IPSR() != 0U)
    {
        return;
    }

    if (RTOS_IsRunning() && debugUartMutexHandle != NULL)
    {
        (void)xSemaphoreGive(debugUartMutexHandle);
    }
}

uint8_t RTOS_LockCanTx(void)
{
    if (__get_IPSR() != 0U)
    {
        return 0U;
    }

    if (RTOS_IsRunning() && canTxMutexHandle != NULL)
    {
        return (xSemaphoreTake(canTxMutexHandle, portMAX_DELAY) == pdPASS) ? 1U : 0U;
    }

    return 0U;
}

void RTOS_UnlockCanTx(void)
{
    if (__get_IPSR() != 0U)
    {
        return;
    }

    if (RTOS_IsRunning() && canTxMutexHandle != NULL)
    {
        (void)xSemaphoreGive(canTxMutexHandle);
    }
}

BaseType_t RTOS_QueueUartLineFromISR(uint8_t port,
                                     const char *line,
                                     BaseType_t *pxHigherPriorityTaskWoken)
{
    UartLineMessage_t msg;

    if (uartLineQueueHandle == NULL || line == NULL)
    {
        return pdFAIL;
    }

    memset(&msg, 0, sizeof(msg));
    msg.port = port;
    strncpy(msg.line, line, sizeof(msg.line) - 1U);
    msg.line[sizeof(msg.line) - 1U] = '\0';

    return xQueueSendFromISR(uartLineQueueHandle, &msg, pxHigherPriorityTaskWoken);
}

void RTOS_GiveCanErrorSemaphoreFromISR(BaseType_t *pxHigherPriorityTaskWoken)
{
    if (canErrorSemaphoreHandle != NULL)
    {
        (void)xSemaphoreGiveFromISR(canErrorSemaphoreHandle, pxHigherPriorityTaskWoken);
    }
}

void Gateway_Task(void *argument)
{
    UartLineMessage_t msg;
    const char *port_name;

    (void)argument;

    Fake_UART_Test_Once();

    for (;;)
    {
        if (xQueueReceive(uartLineQueueHandle, &msg, portMAX_DELAY) == pdPASS)
        {
            port_name = (msg.port == 2U) ? "UART2" : "UART3";
            Process_UART_Line(msg.line, port_name);
        }
    }
}

void CAN_Error_Task(void *argument)
{
    (void)argument;

    for (;;)
    {
        if (xSemaphoreTake(canErrorSemaphoreHandle, portMAX_DELAY) == pdPASS)
        {
            CAN_Print_Error("CAN ERROR SEMAPHORE");
        }
    }
}

void Test_Task(void *argument)
{
    (void)argument;

    for (;;)
    {
        CAN_Ext_Test_Task();
        vTaskDelay(pdMS_TO_TICKS(10U));
    }
}

void Heartbeat_Timer_Callback(TimerHandle_t xTimer)
{
    (void)xTimer;
    UART1_Print("Heartbeat software timer: waiting UART2/UART3...\r\n");
}

void RTOS_InitObjects(void)
{
    uartLineQueueHandle = xQueueCreate(UART_LINE_QUEUE_LENGTH, sizeof(UartLineMessage_t));
    debugUartMutexHandle = xSemaphoreCreateMutex();
    canTxMutexHandle = xSemaphoreCreateMutex();
    canErrorSemaphoreHandle = xSemaphoreCreateBinary();

    heartbeatTimerHandle = xTimerCreate("HeartbeatTimer",
                                        pdMS_TO_TICKS(HEARTBEAT_PERIOD_MS),
                                        pdTRUE,
                                        NULL,
                                        Heartbeat_Timer_Callback);

    if (uartLineQueueHandle == NULL ||
        debugUartMutexHandle == NULL ||
        canTxMutexHandle == NULL ||
        canErrorSemaphoreHandle == NULL ||
        heartbeatTimerHandle == NULL)
    {
        UART1_Print("RTOS object create failed\r\n");
        Error_Handler();
    }

    if (xTaskCreate(Gateway_Task,
                    "GatewayTask",
                    GATEWAY_TASK_STACK_WORDS,
                    NULL,
                    GATEWAY_TASK_PRIORITY,
                    NULL) != pdPASS)
    {
        UART1_Print("Gateway task create failed\r\n");
        Error_Handler();
    }

    if (xTaskCreate(CAN_Error_Task,
                    "CanErrTask",
                    CAN_ERROR_TASK_STACK_WORDS,
                    NULL,
                    CAN_ERROR_TASK_PRIORITY,
                    NULL) != pdPASS)
    {
        UART1_Print("CAN error task create failed\r\n");
        Error_Handler();
    }

#if ENABLE_CAN_EXT_TEST
    if (xTaskCreate(Test_Task,
                    "TestTask",
                    TEST_TASK_STACK_WORDS,
                    NULL,
                    TEST_TASK_PRIORITY,
                    NULL) != pdPASS)
    {
        UART1_Print("Test task create failed\r\n");
        Error_Handler();
    }
#endif

#if HEARTBEAT_ENABLE
    if (xTimerStart(heartbeatTimerHandle, 0U) != pdPASS)
    {
        UART1_Print("Heartbeat software timer start failed\r\n");
        Error_Handler();
    }
#endif
}

void RTOS_StartScheduler(void)
{
    UART1_Print("FreeRTOS scheduler starting\r\n");
    vTaskStartScheduler();

    UART1_Print("FreeRTOS scheduler returned unexpectedly\r\n");
    Error_Handler();
}

#else

void RTOS_InitObjects(void) {}
void RTOS_StartScheduler(void) {}
uint8_t RTOS_IsRunning(void) { return 0U; }

#endif /* APP_USE_FREERTOS */
