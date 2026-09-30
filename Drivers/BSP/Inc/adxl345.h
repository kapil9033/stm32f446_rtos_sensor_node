#ifndef ADXL345_H
#define ADXL345_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

// ADXL345 I2C 7-bit Address (ALT ADDRESS pin LOW -> 0x53) shifted to 8-bit
#define ADXL345_I2C_ADDR         (0x53 << 1)

// Register Addresses
#define ADXL345_REG_DEVID        0x00
#define ADXL345_REG_POWER_CTL    0x2D
#define ADXL345_REG_DATA_FORMAT  0x31
#define ADXL345_REG_DATAX0       0x32

// Configuration Constants
#define ADXL345_DEVID_VAL        0xE5
#define ADXL345_RANGE_2G         0x00
#define ADXL345_RANGE_4G         0x01
#define ADXL345_RANGE_8G         0x02
#define ADXL345_RANGE_16G        0x03
#define ADXL345_FULL_RES         0x08
#define ADXL345_MEASURE_BIT      0x08

// Accelerometer Data Structure
typedef struct {
    int16_t x_raw;
    int16_t y_raw;
    int16_t z_raw;
    float   x_mg;
    float   y_mg;
    float   z_mg;
} ADXL345_Data_t;

// Driver API Functions
HAL_StatusTypeDef ADXL345_Init(I2C_HandleTypeDef *hi2c, uint8_t range);
HAL_StatusTypeDef ADXL345_ReadAccel_DMA(I2C_HandleTypeDef *hi2c, uint8_t *rx_buffer);
void ADXL345_ParseData(const uint8_t *rx_buffer, ADXL345_Data_t *data_struct);

#endif // ADXL345_H
