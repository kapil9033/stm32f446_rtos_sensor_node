#include "max7219.h"

/**
  * @brief  Drive Chip Select (CS/LOAD) Low
  */
inline void MAX7219_CS_Select(MAX7219_HandleTypeDef *hmax) {
    HAL_GPIO_WritePin(hmax->cs_port, hmax->cs_pin, GPIO_PIN_RESET);
}

/**
  * @brief  Drive Chip Select (CS/LOAD) High to latch SPI data
  */
inline void MAX7219_CS_Deselect(MAX7219_HandleTypeDef *hmax) {
    HAL_GPIO_WritePin(hmax->cs_port, hmax->cs_pin, GPIO_PIN_SET);
}

/**
  * @brief  Write a single 16-bit register packet to MAX7219 (Blocking setup mode)
  */
HAL_StatusTypeDef MAX7219_WriteRegister(MAX7219_HandleTypeDef *hmax, uint8_t reg, uint8_t data) {
  uint16_t packet = ((uint16_t)reg << 8) | data;
    HAL_StatusTypeDef status;

    MAX7219_CS_Select(hmax);
  status = HAL_SPI_Transmit(hmax->hspi, (uint8_t *)&packet, 1, 100);
    MAX7219_CS_Deselect(hmax);

    return status;
}

/**
  * @brief  Initialize MAX7219 parameters (Display Test, Intensity, Scan Limit, Shutdown Control)
  */
HAL_StatusTypeDef MAX7219_Init(MAX7219_HandleTypeDef *hmax, uint8_t intensity) {
    HAL_StatusTypeDef status = HAL_OK;

    // Deselect chip by default
    MAX7219_CS_Deselect(hmax);

    // Disable Display Test
    status |= MAX7219_WriteRegister(hmax, MAX7219_REG_DISPLAY_TEST, 0x00);
    
    // Set Scan Limit to 8 digits / 8 rows (0x07)
    status |= MAX7219_WriteRegister(hmax, MAX7219_REG_SCAN_LIMIT, 0x07);
    
    // Disable BCD Decode Mode (Raw Matrix / Segment Bitmaps)
    status |= MAX7219_WriteRegister(hmax, MAX7219_REG_DECODE_MODE, 0x00);
    
    // Set Brightness Level (0x00 to 0x0F)
    status |= MAX7219_WriteRegister(hmax, MAX7219_REG_INTENSITY, (intensity & 0x0F));
    
    // Wake up driver from Shutdown Mode
    status |= MAX7219_WriteRegister(hmax, MAX7219_REG_SHUTDOWN, MAX7219_NORMAL_MODE);

    return status;
}

/**
  * @brief  Transmit 8 display rows/digits non-blocking via SPI DMA.
  * @note   dma_tx_buf must be at least 8 x uint16_t (16 bytes) in length and persistent in RAM.
  */
HAL_StatusTypeDef MAX7219_UpdateFrameBuffer_DMA(MAX7219_HandleTypeDef *hmax, uint8_t *frame_buffer, uint16_t *dma_tx_buf) {
    // Prepare 16-bit word packets: [Register_Addr (8-bit) | Display_Data (8-bit)]
    for (uint8_t i = 0; i < 8; i++) {
        uint8_t reg_addr = MAX7219_REG_DIGIT0 + i;
        dma_tx_buf[i] = (uint16_t)((reg_addr << 8) | frame_buffer[i]);
    }

    MAX7219_CS_Select(hmax);

    // Send 16-bit x 8 transactions over SPI DMA
    return HAL_SPI_Transmit_DMA(hmax->hspi, (uint8_t *)dma_tx_buf, 8);
}
