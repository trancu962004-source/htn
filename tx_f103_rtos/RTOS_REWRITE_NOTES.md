# RTOS rewrite notes for tx_f103

## Original flow
- UART2/UART3 receive one byte by HAL interrupt.
- `HAL_UART_RxCpltCallback()` builds a full line until `\n`.
- `main()` polls `uart2_line_ready` / `uart3_line_ready` and calls `Process_UART_Line()`.
- `Process_UART_Line()` parses VITAL or LOADCELL and sends CAN Extended frame.
- Heartbeat is implemented by polling `HAL_GetTick()` in `Heartbeat_Task()`.

## RTOS flow added
- Task: `Gateway_Task()` waits for UART lines and calls `Process_UART_Line()`.
- Queue: `uartLineQueueHandle` carries `UartLineMessage_t` from UART interrupt to `Gateway_Task()`.
- Mutex: `debugUartMutexHandle` protects USART1 debug prints; `canTxMutexHandle` protects CAN TX.
- Semaphore: `canErrorSemaphoreHandle` is given by CAN error interrupt and consumed by `CAN_Error_Task()`.
- Software timer: `heartbeatTimerHandle` replaces heartbeat polling.
- Interrupt: UART2/UART3 RX callbacks still receive bytes, but full lines are now queued using `xQueueSendFromISR()`.

## Important build note
The original project did not include FreeRTOS middleware. This rewrite sets `APP_USE_FREERTOS` to 1 in `Core/Inc/app_config.h`, so you must enable/add FreeRTOS sources and include paths before building. If you want to compile the original super-loop behavior, set `APP_USE_FREERTOS` to 0.
