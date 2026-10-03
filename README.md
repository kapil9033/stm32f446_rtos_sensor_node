# STM32F446 Sensor Node with FreeRTOS & CMake

## Project Goal

The primary objective of this project is to develop a modular, real-time embedded system on the **STM32F446RE** microcontroller running **FreeRTOS**. 

Key capabilities include:
- Concurrent sensor data acquisition (ADXL335 analog accelerometer, DS3231 RTC).
- A Command Line Interface (CLI) task executing over UART for real-time monitoring and debugging.
- Cross-platform, reproducible build system configuration using **CMake** and `arm-none-eabi-gcc`.
- Vendor driver integration (STM32 HAL & ARM CMSIS) managed using Git Submodules.

---

## Hardware Setup

| Component | Interface / Peripheral | MCU Pin / Connection | Description |
| :--- | :--- | :--- | :--- |
| **STM32F446RE Nucleo-64** | Board / MCU | N/A | Main Controller (Cortex-M4 @ 180MHz) |
| **ADXL335** | Analog outputs / ADC | PA0, PA1, PC0 | 3-Axis Analog Accelerometer |
| **DS3231** | I2C | Standard Peripheral Pins | High-Precision Real-Time Clock (RTC) |
| **MAX7219** | SPI / GPIO | Standard Peripheral Pins | LED Matrix / Display Driver |
| **Debug / CLI Console** | USART / ST-LINK | USB (Virtual COM Port) | Interactive CLI shell and debug output |

## NUCLEO-F446RE Board Pinout

The tables below use the connector labels printed on the Nucleo board. Pin
numbers refer to the connector, not to STM32 package pins.

### Power connector CN6

| CN6 pin | Label | Description |
| ---: | --- | --- |
| 1 | NC | Not connected |
| 2 | IOREF | 3.3 V I/O reference |
| 3 | RESET | MCU reset |
| 4 | +3V3 | 3.3 V power |
| 5 | +5V | 5 V power |
| 6-7 | GND | Ground |
| 8 | VIN | External input voltage |

### Analog connector CN8

| CN8 pin | Arduino label | STM32 pin / function |
| ---: | --- | --- |
| 1 | A0 | PA0 / ADC1_IN0 |
| 2 | A1 | PA1 / ADC1_IN1 |
| 3 | A2 | PA4 / ADC1_IN4 |
| 4 | A3 | PB0 / ADC1_IN8 |
| 5 | A4 | PC1 / ADC1_IN11 |
| 6 | A5 | PC0 / ADC1_IN10 |

### Digital connectors CN5 and CN9

| Connector | Pin | Board label | STM32 pin / function |
| --- | ---: | --- | --- |
| CN5 | 1 | D8 | PA9 |
| CN5 | 2 | D9 | PC7 / TIM3_CH2, TIM8_CH2 |
| CN5 | 3 | D10 | PB6 / SPI1 chip select (GPIO) |
| CN5 | 4 | D11 | PA7 / SPI1_MOSI |
| CN5 | 5 | D12 | PA6 / SPI1_MISO |
| CN5 | 6 | D13 | PA5 / SPI1_SCK, onboard LD2 |
| CN5 | 7 | GND | Ground |
| CN5 | 8 | AVDD | Analog supply reference |
| CN5 | 9 | D14 / SDA | PB9 / I2C1_SDA |
| CN5 | 10 | D15 / SCL | PB8 / I2C1_SCL |
| CN9 | 1 | D0 / RX | PA3 / USART2_RX (ST-LINK VCP) |
| CN9 | 2 | D1 / TX | PA2 / USART2_TX (ST-LINK VCP) |
| CN9 | 3 | D2 | PA10 |
| CN9 | 4 | D3 | PB3 / TIM2_CH2 |
| CN9 | 5 | D4 | PB5 |
| CN9 | 6 | D5 | PB4 / TIM3_CH1 |
| CN9 | 7 | D6 | PB10 / TIM2_CH3 |
| CN9 | 8 | D7 | PA8 |

### ST morpho headers CN7 and CN10

| Pin | CN7 signal | Pin | CN10 signal |
| ---: | --- | ---: | --- |
| 1 | PC10 | 1 | PC9 |
| 2 | PC11 | 2 | PC8 |
| 3 | PC12 | 3 | PB8 |
| 4 | PD2 | 4 | PC6 |
| 5 | VDD | 5 | PB9 |
| 6 | E5V | 6 | PC5 |
| 7 | BOOT0 | 7 | AVDD |
| 8 | GND | 8 | U5V |
| 9 | NC | 9 | GND |
| 10 | NC | 10 | NC |
| 11 | NC | 11 | PA5 |
| 12 | IOREF | 12 | PA12 |
| 13 | NRST | 13 | PA6 |
| 14 | RESET | 14 | PA11 |
| 15 | 3V3 | 15 | PA7 |
| 16 | 3V3 | 16 | PB12 |
| 17 | 5V | 17 | PB6 |
| 18 | 5V | 18 | PB11 |
| 19 | GND | 19 | PC7 |
| 20 | GND | 20 | GND |
| 21 | GND | 21 | PA9 |
| 22 | GND | 22 | PB2 |
| 23 | VIN | 23 | PA8 |
| 24 | NC | 24 | PB1 |
| 25 | NC | 25 | PB10 |
| 26 | PA0 | 26 | PB15 |
| 27 | PA1 | 27 | PB4 |
| 28 | PA4 | 28 | PB14 |
| 29 | PA4 | 29 | PB5 |
| 30 | PB0 | 30 | PB13 |
| 31 | PB0 | 31 | PB3 |
| 32 | PC1 | 32 | AGND |
| 33 | PC1 | 33 | PA10 |
| 34 | PC0 | 34 | PC4 |
| 35 | PC0 | 35 | PA2 |
| 36 | PD2 | 36 | NC |
| 37 | PD2 | 37 | PA3 |
| 38 | PH0 | 38 | NC |

Check the pin names against the silkscreen and the official NUCLEO-F446RE
board documentation before wiring. Board revisions may differ.

---

## Peripheral Wiring

Below is the complete hardware pin connection table connecting the NUCLEO-F446RE board to external peripherals:

### 1. ADXL335 Accelerometer (Analog Interface)

| ADXL335 Pin | STM32F446RE Pin | Signal Function |
| :--- | :--- | :--- |
| **VCC** | 3.3V | Power Supply |
| **GND** | GND | Common Ground |
| **X-OUT** | PA0 | ADC1_IN0 |
| **Y-OUT** | PA1 | ADC1_IN1 |
| **Z-OUT** | PC0 | ADC1_IN10 |

Power the ADXL335 from 3.3V and connect all grounds together. Its analog
outputs must stay within the STM32 ADC input range (0 to 3.3V).

---

### 2. DS3231 RTC Module (I2C Interface)

| DS3231 Pin | STM32F446RE Pin | Signal Function |
| :--- | :--- | :--- |
| **VCC** | 3.3V | Power Supply |
| **GND** | GND | Ground |
| **SDA** | PB9 | I2C1_SDA (4.7 kΩ pull-up to 3.3 V if not provided by module) |
| **SCL** | PB8 | I2C1_SCL (4.7 kΩ pull-up to 3.3 V if not provided by module) |

Connect the DS3231 to I2C1: PB9 is SDA and PB8 is SCL. Power the module from
3.3 V for this setup, connect grounds together, and leave its 32K and SQW pins
unconnected. Use I2C pull-ups to 3.3 V if the module does not already provide
them; do not pull the STM32 I/O lines up to 5 V.

To test the RTC after flashing, open the board's ST-LINK Virtual COM Port at
115200 baud, wait for the `STM32>` prompt, and enter `get-time`. A successful
read prints the time and date. An `[RTC I2C error]` response means the MCU did
not complete the register read; check power, ground, SDA/SCL wiring, and that
the bus pull-ups go to 3.3 V.

The firmware reads the ADXL335 through ADC1 and `get-accel` reports raw ADC
counts and approximate millivolts for each axis. Millivolts use a nominal 3.3V
ADC reference; the command does not calibrate the sensor's zero-g offset or
convert the readings to acceleration units.

---

### 3. MAX7219 Display Module (SPI1 Interface)

| MAX7219 Pin | STM32F446RE Pin | Signal Function |
| :--- | :--- | :--- |
| **VCC** | 5V | Power Supply |
| **GND** | GND | Ground |
| **DIN** | PWM/MOSI/D11 / User Selected | Data Input (SPI MOSI or GPIO) |
| **CS / LOAD** | PWM/CS/D10 / User Selected | Chip Select / Latch |
| **CLK** | SCK/D13 / User Selected | Clock Signal |

---

### 4. Serial Debug & CLI Interface (USART2 / ST-Link VCP)

| Function | STM32F446RE Pin | Description |
| :--- | :--- | :--- |
| **USART2_TX** | PA2 | Transmit to ST-Link VCP / Serial Terminal |
| **USART2_RX** | PA3 | Receive from ST-Link VCP / Serial Terminal |

---

## Getting Started

### Prerequisites

- CMake 3.22 or newer
- `arm-none-eabi-gcc`, `arm-none-eabi-g++`, `arm-none-eabi-objcopy`, and
  `arm-none-eabi-size`
- GNU Make or Ninja
- OpenOCD or STM32CubeProgrammer for flashing

On Ubuntu or Debian, install the build tools with:

```sh
sudo apt update
sudo apt install build-essential cmake gcc-arm-none-eabi \
  binutils-arm-none-eabi gdb-multiarch git
```

### Clone the repository

HAL and CMSIS dependencies are included as Git submodules:

```sh
git clone --recursive git@github.com:kapil9033/stm32f446_rtos_sensor_node.git
cd stm32f446_rtos_sensor_node
```

If the repository was cloned without submodules, initialize them with:

```sh
git submodule update --init --recursive
```

### Build

```sh
cmake -S . -B build -G "Unix Makefiles"
cmake --build build --parallel
```

The `build/` directory contains the ELF, BIN, HEX, and MAP outputs.

### Flash with OpenOCD

With the Nucleo board connected over ST-LINK, run:

```sh
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
  -c "program build/stm32f446_rtos_sensor_node.elf verify reset exit"
```

Open a serial terminal on the ST-LINK Virtual COM Port at 115200 baud and use
`get-time` to check the connected DS3231.

## Startup and Troubleshooting

The firmware includes the STM32F446 startup/vector table and linker script.
Three brief flashes of the Nucleo LD2 LED indicate startup; PA5 is also used
for SPI1 SCK, so the LED is not expected to continue blinking after SPI
initialization. The USART2 CLI is available through the ST-LINK Virtual COM
Port at 115200 baud.

If the board does not start, inspect the vector table and the first two words
of the generated binary:

```sh
arm-none-eabi-objdump -s -j .isr_vector build/stm32f446_rtos_sensor_node.elf
od -An -tx4 -N8 build/stm32f446_rtos_sensor_node.bin
```

The first word should be an SRAM address (`0x200xxxxx`); the second should be
an odd flash address (`0x080xxxxx`). Use the build and OpenOCD commands above
to regenerate and flash the firmware.
