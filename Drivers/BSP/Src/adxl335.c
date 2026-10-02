#include "adxl335.h"

HAL_StatusTypeDef ADXL335_ReadRaw(ADC_HandleTypeDef *hadc, uint32_t samples[3], uint32_t timeout_ms)
{
    static const uint32_t channels[3] = {
        ADC_CHANNEL_0,
        ADC_CHANNEL_1,
        ADC_CHANNEL_10
    };
    ADC_ChannelConfTypeDef channel_config = {0};

    if (hadc == NULL || samples == NULL) {
        return HAL_ERROR;
    }

    channel_config.Rank = 1;
    channel_config.SamplingTime = ADC_SAMPLETIME_480CYCLES;

    for (uint8_t axis = 0; axis < 3; ++axis) {
        HAL_StatusTypeDef status;

        channel_config.Channel = channels[axis];
        status = HAL_ADC_ConfigChannel(hadc, &channel_config);
        if (status != HAL_OK) {
            return status;
        }

        status = HAL_ADC_Start(hadc);
        if (status != HAL_OK) {
            return status;
        }

        status = HAL_ADC_PollForConversion(hadc, timeout_ms);
        if (status == HAL_OK) {
            samples[axis] = HAL_ADC_GetValue(hadc);
        }

        if (HAL_ADC_Stop(hadc) != HAL_OK) {
            return HAL_ERROR;
        }
        if (status != HAL_OK) {
            return status;
        }
    }

    return HAL_OK;
}