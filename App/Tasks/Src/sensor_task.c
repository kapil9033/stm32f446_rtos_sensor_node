#include "sensor_tasks.h"
#include <stdio.h>

// Define Handles
QueueHandle_t xSensorDataQueue = NULL;
SemaphoreHandle_t xI2CBusMutex = NULL;

// Dummy hardware driver stubs (Replace with your actual HAL/LL DMA drivers)
static void DS3231_ReadTime(uint8_t *h, uint8_t *m, uint8_t *s) {
    *h = 12; *m = 34; *s = 56; // Mock RTC time
}

static void ADXL345_ReadAccel(int16_t *x, int16_t *y, int16_t *z) {
    *x = 128; *y = -45; *z = 981; // Mock Acceleration values
}

static void MAX7219_DisplayUpdate(const SensorData_t *data) {
    // Write formatted time or accelerometer values over SPI
    (void)data;
}

/**
 * @brief Periodic Task reading RTC and Accelerometer over shared I2C bus
 */
void vTaskSensors(void *pvParameters) {
    (void)pvParameters;
    SensorData_t sensor_payload;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(100); // Run at 10 Hz

    for (;;) {
        // Enforce deterministic 10Hz execution rate
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        // Acquire Mutex before accessing shared I2C bus (Wait up to 10ms)
        if (xSemaphoreTake(xI2CBusMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            
            // Read I2C Devices
            DS3231_ReadTime(&sensor_payload.hours, &sensor_payload.minutes, &sensor_payload.seconds);
            ADXL345_ReadAccel(&sensor_payload.accel_x, &sensor_payload.accel_y, &sensor_payload.accel_z);
            
            sensor_payload.timestamp_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

            // Release shared bus mutex
            xSemaphoreGive(xI2CBusMutex);

            // Post telemetry data to queue (Non-blocking if queue full)
            if (xQueueSend(xSensorDataQueue, &sensor_payload, 0) != pdPASS) {
                // Queue full error handling (e.g., increment drop counter)
            }
        }
    }
}

/**
 * @brief Consumer Task waiting for Queue data to update display over SPI
 */
void vTaskDisplay(void *pvParameters) {
    (void)pvParameters;
    SensorData_t received_data;

    for (;;) {
        // Block indefinitely until a new sensor measurement is pushed onto queue
        if (xQueueReceive(xSensorDataQueue, &received_data, portMAX_DELAY) == pdTRUE) {
            
            // Render on MAX7219 8x8 LED Matrix / 7-Segment via SPI DMA
            MAX7219_DisplayUpdate(&received_data);
        }
    }
}
