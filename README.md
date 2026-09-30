# STM32F446 Sensor Node with FreeRTOS & CMake

## Project Goal

The primary objective of this project is to develop a modular, real-time embedded system on the **STM32F446RE** microcontroller running **FreeRTOS**. 

Key capabilities include:
- Concurrent sensor data acquisition (ADXL345 accelerometer, DS3231 RTC).
- A Command Line Interface (CLI) task executing over UART for real-time monitoring and debugging.
- Cross-platform, reproducible build system configuration using **CMake** and `arm-none-eabi-gcc`.
- Vendor driver integration (STM32 HAL & ARM CMSIS) managed using Git Submodules.

---

## Hardware Setup

| Component | Interface / Peripheral | MCU Pin / Connection | Description |
| :--- | :--- | :--- | :--- |
| **STM32F446RE Nucleo-64** | Board / MCU | N/A | Main Controller (Cortex-M4 @ 180MHz) |
| **ADXL345** | I2C / SPI | Standard Peripheral Pins | 3-Axis Digital Accelerometer |
| **DS3231** | I2C | Standard Peripheral Pins | High-Precision Real-Time Clock (RTC) |
| **MAX7219** | SPI / GPIO | Standard Peripheral Pins | LED Matrix / Display Driver |
| **Debug / CLI Console** | USART / ST-LINK | USB (Virtual COM Port) | Interactive CLI shell and debug output |

---


---

## Pinout Mapping

Below is the complete hardware pin connection table connecting the NUCLEO-F446RE board to external peripherals:

### 1. ADXL345 Accelerometer (SPI Interface)

| ADXL345 Pin | STM32F446RE Pin | Signal Function |
| :--- | :--- | :--- |
| **VCC** | 3.3V / 5V | Power Supply |
| **GND** | GND | Ground |
| **CS** | PB6 / User Selected | Chip Select (Software GPIO) |
| **SDO** | PA6 | SPI1_MISO |
| **SDA / SDI** | PA7 | SPI1_MOSI |
| **SCL** | PA5 | SPI1_SCK |

---

### 2. DS3231 RTC Module (I2C Interface)

| DS3231 Pin | STM32F446RE Pin | Signal Function |
| :--- | :--- | :--- |
| **VCC** | 3.3V | Power Supply |
| **GND** | GND | Ground |
| **SDA** | PB9 / PB7 | I2C1_SDA (4.7kΩ Pull-up required) |
| **SCL** | PB8 / PB6 | I2C1_SCL (4.7kΩ Pull-up required) |

---

### 3. MAX7219 Display Module (SPI / GPIO Bit-Bang Interface)

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

* `arm-none-eabi-gcc` toolchain
* `cmake` (v3.22 or higher)
* `ninja` or `make`
* `stlink` / `openocd` (for flashing)

### Cloning the Repository

Because this project uses Git submodules for HAL and CMSIS driver dependencies, clone recursively:

```bash
git clone --recursive git@github.com:kapil9033/stm32f446_rtos_sensor_node.git
cd stm32f446_rtos_sensor_node


## Prerequisites & Toolchain Setup

Ensure the following tools are installed on your build host:

- **ARM GNU Toolchain**: `arm-none-eabi-gcc`, `arm-none-eabi-g++`, `arm-none-eabi-objcopy`, `arm-none-eabi-size`
- **Build System**: `cmake` (>= 3.22) and `make` / `gmake`
- **Version Control**: `git`

### Installing Toolchain (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install build-essential cmake gcc-arm-none-eabi binutils-arm-none-eabi gdb-multiarch git


### Repository Cloning
Since vendor drivers (HAL, CMSIS) are integrated as Git submodules, clone the repository recursively:

Bash
git clone --recursive git@github.com:kapil9033/stm32f446_rtos_sensor_node.git
cd stm32f446_rtos_sensor_node

# If you already cloned without --recursive, initialize submodules using:

Bash
git submodule update --init --recursive
### Building the Project
## Generate Build System:

Bash
cmake -B build -G "Unix Makefiles"
# Compile Project:

Bash
cmake --build build -j$(nproc)
### Generated Artifacts:
## Upon a successful build, the following files will be available inside the build/ directory:

stm32f446_rtos_sensor_node.elf: ELF executable containing full symbol information.

stm32f446_rtos_sensor_node.bin: Raw binary payload ready for flashing.

stm32f446_rtos_sensor_node.hex: Intel HEX file format.

stm32f446_rtos_sensor_node.map: Memory usage mapping layout.

### Flashing & Debugging
You can flash the generated binary using OpenOCD or STM32CubeProgrammer:

### Flashing with OpenOCD
Bash
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
  -c "program build/stm32f446_rtos_sensor_node.elf verify reset exit"
