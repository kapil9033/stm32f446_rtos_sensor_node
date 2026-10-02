#ifndef ADXL335_H
#define ADXL335_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

HAL_StatusTypeDef ADXL335_ReadRaw(ADC_HandleTypeDef *hadc, uint32_t samples[3], uint32_t timeout_ms);

#endif