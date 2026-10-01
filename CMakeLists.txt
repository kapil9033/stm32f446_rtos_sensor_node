cmake_minimum_required(VERSION 3.22)

# Bare-metal cross-compilation setup
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

# Toolchain setup
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_SIZE arm-none-eabi-size)

project(stm32f446_rtos_sensor_node C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# MCU Flags for STM32F446RE (Cortex-M4 with Hard FPU)
set(CPU_PARAMETERS
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
)

# Compiler & Linker Options
add_compile_options(
    ${CPU_PARAMETERS} 
    -DSTM32F446xx 
    -DUSE_HAL_DRIVER 
    -Wall 
    -Wextra 
    -fdata-sections 
    -ffunction-sections
)

add_link_options(
    ${CPU_PARAMETERS} 
    -Wl,--gc-sections 
    -specs=nano.specs 
    -specs=nosys.specs 
    -Wl,-T,${CMAKE_SOURCE_DIR}/STM32F446RETx_FLASH.ld
    -Wl,-Map=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map
)

# Include Directories
include_directories(
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${CMAKE_SOURCE_DIR}/App/Tasks/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/BSP/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Include
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include
    ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include
    ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
    ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
)

# Core Sources
file(GLOB_RECURSE CORE_SOURCES "${CMAKE_SOURCE_DIR}/Core/Src/*.c")
file(GLOB_RECURSE APP_SOURCES "${CMAKE_SOURCE_DIR}/App/Tasks/Src/*.c")
file(GLOB_RECURSE BSP_SOURCES "${CMAKE_SOURCE_DIR}/Drivers/BSP/Src/*.c")

# HAL Driver Sources (Filter out *_template.c files)
file(GLOB HAL_SOURCES "${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal*.c")
list(FILTER HAL_SOURCES EXCLUDE REGEX ".*_template\\.c$")

# CMSIS System Source File (Defines SystemCoreClock & AHBPrescTable)
file(GLOB_RECURSE CMSIS_SOURCES "${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c")
set(STARTUP_SOURCE
    "${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f446xx.s"
)

# FreeRTOS Sources
file(GLOB FREERTOS_SOURCES "${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/*.c")
set(FREERTOS_PORT_SOURCES
    "${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c"
    "${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c"
)

# Combine all source files
set(SOURCES
    ${STARTUP_SOURCE}
    ${CORE_SOURCES}
    ${APP_SOURCES}
    ${BSP_SOURCES}
    ${HAL_SOURCES}
    ${CMSIS_SOURCES}
    ${FREERTOS_SOURCES}
    ${FREERTOS_PORT_SOURCES}
)

add_executable(${PROJECT_NAME}.elf ${SOURCES})
set_property(TARGET ${PROJECT_NAME}.elf APPEND PROPERTY LINK_DEPENDS
    "${CMAKE_SOURCE_DIR}/STM32F446RETx_FLASH.ld"
)

# Post-build binary & size generation
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
)
