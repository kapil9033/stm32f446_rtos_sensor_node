#ifndef CLI_TASK_H
#define CLI_TASK_H

#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdint.h>
#include <stdbool.h>

#define CLI_RING_BUF_SIZE   128
#define CLI_CMD_BUF_SIZE    64

// Circular Ring Buffer Structure
typedef struct {
    uint8_t buffer[CLI_RING_BUF_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
} RingBuffer_t;

// Extern Handles
extern UART_HandleTypeDef huart2;
extern RingBuffer_t xUartRxRingBuf;

// API Prototypes
void CLI_RingBuffer_Put(uint8_t data);
bool CLI_RingBuffer_Get(uint8_t *data);
void vTaskCLI(void *pvParameters);

#endif // CLI_TASK_H
