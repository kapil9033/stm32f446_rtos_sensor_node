#include "cli_task.h"
#include "sensor_tasks.h"
#include "ds3231.h"
#include "adxl345.h"
#include <stdio.h>
#include <string.h>

extern I2C_HandleTypeDef hi2c1;

// Instantiate Ring Buffer
RingBuffer_t xUartRxRingBuf = { .head = 0, .tail = 0 };

/**
  * @brief Push single byte into ring buffer (Called inside UART ISR)
  */
void CLI_RingBuffer_Put(uint8_t data) {
    uint16_t next = (xUartRxRingBuf.head + 1) % CLI_RING_BUF_SIZE;
    if (next != xUartRxRingBuf.tail) { // Avoid buffer overflow
        xUartRxRingBuf.buffer[xUartRxRingBuf.head] = data;
        xUartRxRingBuf.head = next;
    }
}

/**
  * @brief Pop single byte from ring buffer (Called from CLI Task)
  */
bool CLI_RingBuffer_Get(uint8_t *data) {
    if (xUartRxRingBuf.head == xUartRxRingBuf.tail) {
        return false; // Buffer empty
    }
    *data = xUartRxRingBuf.buffer[xUartRxRingBuf.tail];
    xUartRxRingBuf.tail = (xUartRxRingBuf.tail + 1) % CLI_RING_BUF_SIZE;
    return true;
}

/**
  * @brief Transmit string over UART2 (Blocking for CLI responses)
  */
static void CLI_Print(const char *str) {
    HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen(str), 200);
}

/**
  * @brief Process incoming command string
  */
static void CLI_ProcessCommand(char *cmd) {
    char out_buf[256];

    if (strcmp(cmd, "help") == 0) {
        CLI_Print("\r\n--- STM32 FreeRTOS CLI Commands ---\r\n");
        CLI_Print("  help      - Print list of commands\r\n");
        CLI_Print("  get-time  - Read DS3231 RTC Time\r\n");
        CLI_Print("  get-accel - Read ADXL345 Acceleration\r\n");
        CLI_Print("  stats     - Print FreeRTOS Task Runtime Stats\r\n\r\n");
    } 
    else if (strcmp(cmd, "get-time") == 0) {
        if (xSemaphoreTake(xI2CBusMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            uint8_t raw_time[7];
            DS3231_Time_t t;
            HAL_I2C_Mem_Read(&hi2c1, DS3231_I2C_ADDR, DS3231_REG_SECONDS, I2C_MEMADD_SIZE_8BIT, raw_time, 7, 100);
            DS3231_ParseTime(raw_time, &t);
            xSemaphoreGive(xI2CBusMutex);

            snprintf(out_buf, sizeof(out_buf), "\r\n[RTC Time] %02d:%02d:%02d | Date: 20%02d-%02d-%02d\r\n", 
                     t.hours, t.minutes, t.seconds, t.year, t.month, t.date);
            CLI_Print(out_buf);
        } else {
            CLI_Print("\r\n[Error] I2C Bus Busy\r\n");
        }
    } 
    else if (strcmp(cmd, "get-accel") == 0) {
        if (xSemaphoreTake(xI2CBusMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            uint8_t raw_accel[6];
            ADXL345_Data_t accel;
            HAL_I2C_Mem_Read(&hi2c1, ADXL345_I2C_ADDR, ADXL345_REG_DATAX0, I2C_MEMADD_SIZE_8BIT, raw_accel, 6, 100);
            ADXL345_ParseData(raw_accel, &accel);
            xSemaphoreGive(xI2CBusMutex);

            snprintf(out_buf, sizeof(out_buf), "\r\n[Accel mg] X: %.1f mg | Y: %.1f mg | Z: %.1f mg\r\n", 
                     accel.x_mg, accel.y_mg, accel.z_mg);
            CLI_Print(out_buf);
        } else {
            CLI_Print("\r\n[Error] I2C Bus Busy\r\n");
        }
    } 
    else if (strcmp(cmd, "stats") == 0) {
        CLI_Print("\r\nTask          State  Prio  Stack  Num\r\n");
        CLI_Print("-------------------------------------\r\n");
        vTaskList(out_buf);
        CLI_Print(out_buf);
        CLI_Print("\r\n");
    } 
    else if (strlen(cmd) > 0) {
        CLI_Print("\r\nUnknown command. Type 'help' for available commands.\r\n");
    }

    CLI_Print("STM32> ");
}

/**
  * @brief Task waiting on UART Ring Buffer bytes to construct command strings
  */
void vTaskCLI(void *pvParameters) {
    (void)pvParameters;
    char cmd_buf[CLI_CMD_BUF_SIZE];
    uint8_t cmd_idx = 0;
    uint8_t rx_byte = 0;

    CLI_Print("\r\nSystem Initialized. Type 'help' for commands.\r\nSTM32> ");

    for (;;) {
        // Poll ring buffer every 20ms
        while (CLI_RingBuffer_Get(&rx_byte)) {
            // Echo back character to console
            HAL_UART_Transmit(&huart2, &rx_byte, 1, 10);

            // Carriage return or newline triggers command parsing
            if (rx_byte == '\r' || rx_byte == '\n') {
                cmd_buf[cmd_idx] = '\0';
                if (cmd_idx > 0) {
                    CLI_ProcessCommand(cmd_buf);
                    cmd_idx = 0;
                } else {
                    CLI_Print("\r\nSTM32> ");
                }
            } 
            // Backspace handling
            else if (rx_byte == '\b' || rx_byte == 0x7F) {
                if (cmd_idx > 0) {
                    cmd_idx--;
                }
            } 
            // Append character to command buffer
            else if (cmd_idx < (CLI_CMD_BUF_SIZE - 1)) {
                cmd_buf[cmd_idx++] = (char)rx_byte;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
