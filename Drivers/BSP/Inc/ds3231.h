#ifndef DS3231_H
#define DS3231_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

// DS3231 I2C 7-bit Address shift to 8-bit format
#define DS3231_I2C_ADDR         (0x68 << 1)

// Register Map
#define DS3231_REG_SECONDS      0x00
#define DS3231_REG_MINUTES      0x01
#define DS3231_REG_HOURS        0x02
#define DS3231_REG_DAY          0x03
#define DS3231_REG_DATE         0x04
#define DS3231_REG_MONTH        0x05
#define DS3231_REG_YEAR         0x06
#define DS3231_REG_TEMP_MSB     0x11
#define DS3231_REG_TEMP_LSB     0x12

// Time Data Structure
typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint8_t year;
} DS3231_Time_t;

// API Function Prototypes
HAL_StatusTypeDef DS3231_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef DS3231_ReadTime_DMA(I2C_HandleTypeDef *hi2c, uint8_t *rx_buffer);
void DS3231_ParseTime(const uint8_t *rx_buffer, DS3231_Time_t *time_struct);
float DS3231_ReadTemperature(I2C_HandleTypeDef *hi2c);

#endif // DS3231_H
