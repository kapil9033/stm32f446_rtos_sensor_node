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

---


---

## Pinout Mapping

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

The DS3231 connects to I2C1 on PB9 (SDA) and PB8 (SCL). Use pull-ups to 3.3V
if they are not already provided by the RTC module; do not pull STM32 pins up
to 5V.

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

## Problem and Solution

### Problem

After flashing, the NUCLEO-F446RE user LED did not blink and the USART2 terminal
remained silent. The firmware image was not a valid STM32 flash image: the build
did not include the STM32F446 startup/vector-table assembly or a device linker
script. The resulting binary did not place the initial stack pointer and reset
handler at the flash base address (`0x08000000`). A read of the first flash
words returned ELF metadata instead of Cortex-M vectors, so the MCU could not
reach `main()`.

In addition, `main()` had been temporarily replaced with an LED-only infinite
loop. Even with a valid reset path, that version would not initialize USART2
or start FreeRTOS and the CLI. Later diagnostic calls used an undefined
`DBG_PRINT`, which stopped the build; two calls were also placed before USART2
initialization. The boot LED test then exposed a SysTick ownership conflict:
FreeRTOS had replaced the HAL SysTick handler, so `HAL_Delay()` in the
pre-scheduler LED blink never advanced the HAL tick. Startup stalled before
USART2 was initialized.

### Solution

- Added the STM32F446 startup source and `STM32F446RETx_FLASH.ld`, with the
  vector table at `0x08000000`, 512 KiB of flash, and 128 KiB of SRAM.
- Restored application initialization in `main()`: HAL and system clock,
  peripherals, FreeRTOS synchronization objects, sensor/display/CLI tasks, and
  the scheduler. Added the I2C and SPI pin/clock setup required by HAL.
- Replaced the undefined debug-print calls with a USART2-backed helper and
  initialize USART2 before sending diagnostic messages. The current clock
  setup uses the internal HSI source.
- Added a SysTick dispatcher that always advances the HAL tick and advances
  the FreeRTOS tick only after the scheduler starts. This allows startup
  delays and HAL timeout handling to work before the scheduler, without
  stopping RTOS ticks afterward.
- Added three brief LD2 (PA5) flashes at startup as a boot indicator. PA5 is
  also SPI1 SCK, so it is not expected to keep blinking after SPI initialization.

Build the firmware and verify its vector table before flashing:

```sh
cmake -S . -B build
cmake --build build -j$(nproc)
arm-none-eabi-objdump -s -j .isr_vector build/stm32f446_rtos_sensor_node.elf
od -An -tx4 -N8 build/stm32f446_rtos_sensor_node.bin
```

The first binary word must be an SRAM address (`0x200xxxxx`); the second must
be an odd flash address (`0x080xxxxx`). For the verified build from this fix,
the words were `0x20020000` and `0x08001d51`. These addresses may change after
subsequent code changes.

Flash the generated binary as raw binary data at the flash base (do not pass an
ELF file with the `bin` format):

```sh
openocd -f board/st_nucleo_f4.cfg \
  -c "init" -c "reset halt" \
  -c "flash write_image erase /home/stm32f446_rtos_sensor_node.bin 0x08000000 bin" \
  -c "reset run" -c "shutdown"
```

At 115200 baud, the USART2 terminal should print the CLI startup banner and
`STM32>` prompt. The firmware build and vector-table checks passed; the final
hardware flash and terminal output must be confirmed on the connected board.
