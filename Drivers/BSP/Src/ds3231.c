#include "ds3231.h"

// Helper BCD Conversion Utility Functions
static uint8_t BCD2DEC(uint8_t val) {
    return (uint8_t)(((val >> 4) * 10) + (val & 0x0F));
}

static uint8_t DEC2BCD(uint8_t val) {
    return (uint8_t)(((val / 10) << 4) | (val % 10));
}

/**
  * @brief  Verify communication with DS3231
  */
HAL_StatusTypeDef DS3231_Init(I2C_HandleTypeDef *hi2c) {
    return HAL_I2C_IsDeviceReady(hi2c, DS3231_I2C_ADDR, 3, 100);
}

/**
  * @brief  Initiate Non-Blocking DMA Read of 7 Time Registers (0x00 to 0x06)
  */
HAL_StatusTypeDef DS3231_ReadTime_DMA(I2C_HandleTypeDef *hi2c, uint8_t *rx_buffer) {
    return HAL_I2C_Mem_Read_DMA(
        hi2c,
        DS3231_I2C_ADDR,
        DS3231_REG_SECONDS,
        I2C_MEMADD_SIZE_8BIT,
        rx_buffer,
        7
    );
}

/**
  * @brief  Parse raw DMA BCD buffer into time structure
  */
void DS3231_ParseTime(const uint8_t *rx_buffer, DS3231_Time_t *time_struct) {
    time_struct->seconds = BCD2DEC(rx_buffer[0] & 0x7F);
    time_struct->minutes = BCD2DEC(rx_buffer[1] & 0x7F);
    time_struct->hours   = BCD2DEC(rx_buffer[2] & 0x3F); // 24-hour mode
    time_struct->day     = BCD2DEC(rx_buffer[3] & 0x07);
    time_struct->date    = BCD2DEC(rx_buffer[4] & 0x3F);
    time_struct->month   = BCD2DEC(rx_buffer[5] & 0x1F);
    time_struct->year    = BCD2DEC(rx_buffer[6]);
}

/**
  * @brief  Blocking read for temperature sensor
  */
float DS3231_ReadTemperature(I2C_HandleTypeDef *hi2c) {
    uint8_t temp_raw[2] = {0};
    if (HAL_I2C_Mem_Read(hi2c, DS3231_I2C_ADDR, DS3231_REG_TEMP_MSB, I2C_MEMADD_SIZE_8BIT, temp_raw, 2, 100) == HAL_OK) {
        int8_t msb = (int8_t)temp_raw[0];
        uint8_t lsb = temp_raw[1] >> 6;
        return (float)msb + (lsb * 0.25f);
    }
    return 0.0f;
}
