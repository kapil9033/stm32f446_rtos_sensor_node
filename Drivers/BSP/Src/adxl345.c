#include "adxl345.h"

// ADXL345 sensitivity in Full-Res mode is ~3.9 mg/LSB across all ranges
#define ADXL345_SCALE_FACTOR_MG  3.9f

/**
  * @brief  Verify Device ID and set POWER_CTL & DATA_FORMAT registers
  */
HAL_StatusTypeDef ADXL345_Init(I2C_HandleTypeDef *hi2c, uint8_t range) {
    uint8_t dev_id = 0;
    HAL_StatusTypeDef status;

    // 1. Verify Device ID
    status = HAL_I2C_Mem_Read(hi2c, ADXL345_I2C_ADDR, ADXL345_REG_DEVID, I2C_MEMADD_SIZE_8BIT, &dev_id, 1, 100);
    if (status != HAL_OK || dev_id != ADXL345_DEVID_VAL) {
        return HAL_ERROR;
    }

    // 2. Configure Data Format: Enable Full-Resolution mode and set measuring range
    uint8_t data_format = ADXL345_FULL_RES | (range & 0x03);
    status = HAL_I2C_Mem_Write(hi2c, ADXL345_I2C_ADDR, ADXL345_REG_DATA_FORMAT, I2C_MEMADD_SIZE_8BIT, &data_format, 1, 100);
    if (status != HAL_OK) {
        return status;
    }

    // 3. Set Measure bit in POWER_CTL register to start sampling
    uint8_t power_ctl = ADXL345_MEASURE_BIT;
    status = HAL_I2C_Mem_Write(hi2c, ADXL345_I2C_ADDR, ADXL345_REG_POWER_CTL, I2C_MEMADD_SIZE_8BIT, &power_ctl, 1, 100);

    return status;
}

/**
  * @brief  Initiate Non-Blocking DMA Read of 6 raw axis registers (DATAX0 to DATAZ1)
  */
HAL_StatusTypeDef ADXL345_ReadAccel_DMA(I2C_HandleTypeDef *hi2c, uint8_t *rx_buffer) {
    return HAL_I2C_Mem_Read_DMA(
        hi2c,
        ADXL345_I2C_ADDR,
        ADXL345_REG_DATAX0,
        I2C_MEMADD_SIZE_8BIT,
        rx_buffer,
        6
    );
}

/**
  * @brief  Parse raw DMA buffer into signed 16-bit integers and physical milli-g
  */
void ADXL345_ParseData(const uint8_t *rx_buffer, ADXL345_Data_t *data_struct) {
    // Combine LSB and MSB for each axis (Little-Endian)
    data_struct->x_raw = (int16_t)((rx_buffer[1] << 8) | rx_buffer[0]);
    data_struct->y_raw = (int16_t)((rx_buffer[3] << 8) | rx_buffer[2]);
    data_struct->z_raw = (int16_t)((rx_buffer[5] << 8) | rx_buffer[4]);

    // Calculate physical values in milli-g (mg)
    data_struct->x_mg = (float)data_struct->x_raw * ADXL345_SCALE_FACTOR_MG;
    data_struct->y_mg = (float)data_struct->y_raw * ADXL345_SCALE_FACTOR_MG;
    data_struct->z_mg = (float)data_struct->z_raw * ADXL345_SCALE_FACTOR_MG;
}
