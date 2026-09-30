#ifndef SENSOR_TASKS_H
#define SENSOR_TASKS_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdint.h>

// Structured payload passed over the Queue
typedef struct {
    uint8_t  hours;
    uint8_t  minutes;
    uint8_t  seconds;
    int16_t  accel_x;
    int16_t  accel_y;
    int16_t  accel_z;
    uint32_t timestamp_ms;
} SensorData_t;

// Global IPC Handles
extern QueueHandle_t xSensorDataQueue;
extern SemaphoreHandle_t xI2CBusMutex;

// Task Prototypes
void vTaskSensors(void *pvParameters);
void vTaskDisplay(void *pvParameters);

#endif // SENSOR_TASKS_H
