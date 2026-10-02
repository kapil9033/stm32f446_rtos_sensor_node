#include "main.h"
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "string.h"
#include "ds3231.h"
#include "adxl335.h"
#include "max7219.h"
#include "sensor_tasks.h"
#include "cli_task.h"

/* Peripheral Handles */
I2C_HandleTypeDef hi2c1;
DMA_HandleTypeDef hdma_i2c1_rx;
DMA_HandleTypeDef hdma_i2c1_tx;
ADC_HandleTypeDef hadc1;

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;

UART_HandleTypeDef huart2;
static uint8_t rx_byte_isr;

MAX7219_HandleTypeDef hmax7219;

/* Private Function Prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
static void Boot_Status_Blink(void);
static void Debug_Print(const char *message);
static void MAX7219_RunSelfTest(void);
static void vTaskAlphabetDisplay(void *pvParameters);
void xPortSysTickHandler(void);
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    /* Trap CPU on stack overflow */
    taskDISABLE_INTERRUPTS();
    for (;;);
}

void Error_Handler(void) {
    __disable_irq();
    while (1) {
        /* Trap CPU on fatal errors */
    }
}

void SysTick_Handler(void)
{
  HAL_IncTick();
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    xPortSysTickHandler();
  }
}

/**
  * @brief Application Entry Point
  */

int main(void)
{
    HAL_Init();
    Boot_Status_Blink();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_DMA_Init();
    MX_ADC1_Init();
    MX_USART2_UART_Init();
    Debug_Print("\r\nUSART2 initialized\r\n");

    MX_I2C1_Init();
    Debug_Print("I2C1 initialized\r\n");
    MX_SPI1_Init();
    Debug_Print("SPI1 initialized\r\n");

    hmax7219.hspi = &hspi1;
    hmax7219.cs_port = GPIOB;
    hmax7219.cs_pin = GPIO_PIN_6;
    MAX7219_RunSelfTest();

    DS3231_Init(&hi2c1);

    xI2CBusMutex = xSemaphoreCreateMutex();
    xSensorDataQueue = xQueueCreate(5, sizeof(SensorData_t));
    if (xI2CBusMutex == NULL || xSensorDataQueue == NULL) {
        Error_Handler();
    }

    if (xTaskCreate(vTaskSensors, "SensorsTask", 512, NULL, 2, NULL) != pdPASS ||
        xTaskCreate(vTaskDisplay, "DisplayTask", 512, NULL, 1, NULL) != pdPASS ||
      xTaskCreate(vTaskAlphabetDisplay, "AlphabetTask", 512, NULL, 1, NULL) != pdPASS ||
        xTaskCreate(vTaskCLI, "CLITask", 512, NULL, 3, NULL) != pdPASS) {
        Error_Handler();
    }

    vTaskStartScheduler();
    Error_Handler();
}

static void MX_ADC1_Init(void)
{
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.NbrOfDiscConversion = 0;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;

  if (HAL_ADC_Init(&hadc1) != HAL_OK) {
    Error_Handler();
  }
}

  static void Debug_Print(const char *message)
  {
    HAL_UART_Transmit(&huart2, (uint8_t *)message, (uint16_t)strlen(message), HAL_MAX_DELAY);
  }

  static void MAX7219_RunSelfTest(void)
  {
    static const uint8_t x_pattern[8] = {
      0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81
    };

    if (MAX7219_Init(&hmax7219, 0x07) != HAL_OK ||
      MAX7219_WriteRegister(&hmax7219, MAX7219_REG_DISPLAY_TEST, 0x01) != HAL_OK) {
      Error_Handler();
    }

    Debug_Print("MAX7219 display test: all LEDs for 500 ms\r\n");
    HAL_Delay(500);

    if (MAX7219_WriteRegister(&hmax7219, MAX7219_REG_DISPLAY_TEST, 0x00) != HAL_OK) {
      Error_Handler();
    }

    for (uint8_t row = 0; row < 8; ++row) {
      if (MAX7219_WriteRegister(&hmax7219, MAX7219_REG_DIGIT0 + row, x_pattern[row]) != HAL_OK) {
        Error_Handler();
      }
    }

    Debug_Print("MAX7219 test pattern sent\r\n");
  }

  static void vTaskAlphabetDisplay(void *pvParameters)
  {
    static const uint8_t alphabet[26][8] = {
      {0x18, 0x24, 0x42, 0x42, 0x7E, 0x42, 0x42, 0x00},
      {0x7C, 0x42, 0x42, 0x7C, 0x42, 0x42, 0x7C, 0x00},
      {0x3C, 0x42, 0x40, 0x40, 0x40, 0x42, 0x3C, 0x00},
      {0x78, 0x44, 0x42, 0x42, 0x42, 0x44, 0x78, 0x00},
      {0x7E, 0x40, 0x40, 0x7C, 0x40, 0x40, 0x7E, 0x00},
      {0x7E, 0x40, 0x40, 0x7C, 0x40, 0x40, 0x40, 0x00},
      {0x3C, 0x42, 0x40, 0x4E, 0x42, 0x42, 0x3C, 0x00},
      {0x42, 0x42, 0x42, 0x7E, 0x42, 0x42, 0x42, 0x00},
      {0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00},
      {0x1E, 0x04, 0x04, 0x04, 0x44, 0x44, 0x38, 0x00},
      {0x42, 0x44, 0x48, 0x70, 0x48, 0x44, 0x42, 0x00},
      {0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7E, 0x00},
      {0x42, 0x66, 0x5A, 0x5A, 0x42, 0x42, 0x42, 0x00},
      {0x42, 0x62, 0x52, 0x4A, 0x46, 0x42, 0x42, 0x00},
      {0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00},
      {0x7C, 0x42, 0x42, 0x7C, 0x40, 0x40, 0x40, 0x00},
      {0x3C, 0x42, 0x42, 0x42, 0x4A, 0x44, 0x3A, 0x00},
      {0x7C, 0x42, 0x42, 0x7C, 0x48, 0x44, 0x42, 0x00},
      {0x3C, 0x42, 0x40, 0x3C, 0x02, 0x42, 0x3C, 0x00},
      {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00},
      {0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00},
      {0x42, 0x42, 0x42, 0x42, 0x24, 0x24, 0x18, 0x00},
      {0x42, 0x42, 0x42, 0x5A, 0x5A, 0x66, 0x42, 0x00},
      {0x42, 0x24, 0x18, 0x18, 0x18, 0x24, 0x42, 0x00},
      {0x42, 0x24, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00},
      {0x7E, 0x02, 0x04, 0x18, 0x20, 0x40, 0x7E, 0x00}
    };

    (void)pvParameters;

    for (;;) {
      for (uint8_t letter = 0; letter < 26; ++letter) {
        for (uint8_t row = 0; row < 8; ++row) {
          if (MAX7219_WriteRegister(&hmax7219, MAX7219_REG_DIGIT0 + row, alphabet[letter][row]) != HAL_OK) {
            Error_Handler();
          }
        }
        vTaskDelay(pdMS_TO_TICKS(750));
      }
    }
  }

static void Boot_Status_Blink(void)
{
    GPIO_InitTypeDef gpio_led = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    gpio_led.Pin = GPIO_PIN_5;
    gpio_led.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_led.Pull = GPIO_NOPULL;
    gpio_led.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &gpio_led);

    for (uint8_t blink = 0; blink < 3; ++blink) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        HAL_Delay(100);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        HAL_Delay(100);
    }
}

/**
  * @brief System Clock Configuration (180 MHz Core Clock from HSI)
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators: Enable HSI and configure PLL */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes CPU, AHB and APB buses clocks */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief DMA Controller Initialization
  */
static void MX_DMA_Init(void) {
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();

    HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);

    HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);

    HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
}

/**
  * @brief I2C1 Initialization (PB8 -> SCL, PB9 -> SDA)
  */
static void MX_I2C1_Init(void) {
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 400000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        while(1);
    }
}

/**
  * @brief SPI1 Initialization (PA5 -> SCK, PA7 -> MOSI, PB6 -> CS)
  */
static void MX_SPI1_Init(void) {
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_16BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;
    if (HAL_SPI_Init(&hspi1) != HAL_OK) {
        while(1);
    }
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1) {
        GPIO_InitTypeDef gpio = {0};

        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_I2C1_CLK_ENABLE();

        gpio.Pin = GPIO_PIN_8 | GPIO_PIN_9;
        gpio.Mode = GPIO_MODE_AF_OD;
        gpio.Pull = GPIO_PULLUP;
        gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        gpio.Alternate = GPIO_AF4_I2C1;
        HAL_GPIO_Init(GPIOB, &gpio);
    }
}

void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
  if (hadc->Instance == ADC1) {
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    gpio.Mode = GPIO_MODE_ANALOG;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &gpio);

    gpio.Pin = GPIO_PIN_0;
    HAL_GPIO_Init(GPIOC, &gpio);
  }
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1) {
        GPIO_InitTypeDef gpio = {0};

        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_SPI1_CLK_ENABLE();

        gpio.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
        gpio.Mode = GPIO_MODE_AF_PP;
        gpio.Pull = GPIO_NOPULL;
        gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        gpio.Alternate = GPIO_AF5_SPI1;
        HAL_GPIO_Init(GPIOA, &gpio);
    }
}

void HAL_UART_MspInit(UART_HandleTypeDef* huart) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (huart->Instance == USART2) {
        /* Enable Peripheral Clocks */
        __HAL_RCC_USART2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        /**USART2 GPIO Configuration
        PA2     ------> USART2_TX
        PA3     ------> USART2_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
    }

/**
  * @brief USART2 Initialization (PA2 -> TX, PA3 -> RX)
  */
static void MX_USART2_UART_Init(void) {
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK) {
        while(1);
    }

    HAL_NVIC_SetPriority(USART2_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);

    HAL_UART_Receive_IT(&huart2, &rx_byte_isr, 1);
}

/**
  * @brief GPIO Initialization
  */
static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);

    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
  * @brief SPI DMA Transmission Complete Callback
  */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI1) {
        MAX7219_CS_Deselect(&hmax7219);
    }
}

/**
  * @brief UART RX Complete Callback
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
        CLI_RingBuffer_Put(rx_byte_isr);
        HAL_UART_Receive_IT(&huart2, &rx_byte_isr, 1);
    }
}

/**
  * @brief USART2 Interrupt Handler
  */
void USART2_IRQHandler(void) {
    HAL_UART_IRQHandler(&huart2);
}

void vApplicationIdleHook(void) {
}
