#ifndef MAX7219_H
#define MAX7219_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

// MAX7219 Register Addresses
#define MAX7219_REG_NOOP         0x00
#define MAX7219_REG_DIGIT0       0x01
#define MAX7219_REG_DIGIT1       0x02
#define MAX7219_REG_DIGIT2       0x03
#define MAX7219_REG_DIGIT3       0x04
#define MAX7219_REG_DIGIT4       0x05
#define MAX7219_REG_DIGIT5       0x06
#define MAX7219_REG_DIGIT6       0x07
#define MAX7219_REG_DIGIT7       0x08
#define MAX7219_REG_DECODE_MODE  0x09
#define MAX7219_REG_INTENSITY    0x0A
#define MAX7219_REG_SCAN_LIMIT   0x0B
#define MAX7219_REG_SHUTDOWN     0x0C
#define MAX7219_REG_DISPLAY_TEST 0x0F

// Shutdown register options
#define MAX7219_SHUTDOWN_MODE    0x00
#define MAX7219_NORMAL_MODE      0x01

// Driver Handles Structure
typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef      *cs_port;
    uint16_t           cs_pin;
} MAX7219_HandleTypeDef;

// API Prototypes
HAL_StatusTypeDef MAX7219_Init(MAX7219_HandleTypeDef *hmax, uint8_t intensity);
HAL_StatusTypeDef MAX7219_WriteRegister(MAX7219_HandleTypeDef *hmax, uint8_t reg, uint8_t data);
HAL_StatusTypeDef MAX7219_UpdateFrameBuffer_DMA(MAX7219_HandleTypeDef *hmax, uint8_t *frame_buffer, uint16_t *dma_tx_buf);
void MAX7219_CS_Select(MAX7219_HandleTypeDef *hmax);
void MAX7219_CS_Deselect(MAX7219_HandleTypeDef *hmax);

#endif // MAX7219_H
