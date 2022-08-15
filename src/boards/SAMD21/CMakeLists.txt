##
##   ______                              _
##  / _____)             _              | |
## ( (____  _____ ____ _| |_ _____  ____| |__
##  \____ \| ___ |    (_   _) ___ |/ ___)  _ \
##  _____) ) ____| | | || |_| ____( (___| | | |
## (______/|_____)_|_|_| \__)_____)\____)_| |_|
## (C)2013-2017 Semtech
##  ___ _____ _   ___ _  _____ ___  ___  ___ ___
## / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
## \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
## |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
## embedded.connectivity.solutions.==============
##
## License:  Revised BSD License, see LICENSE.TXT file included in the project
## Authors:  Johannes Bruder (STACKFORCE), Miguel Luis (Semtech) and 
##           Marten Lootsma(TWTG) on behalf of Microchip/Atmel (c)2017
##
project(SAMD21)
cmake_minimum_required(VERSION 3.6)
enable_language(ASM)

#---------------------------------------------------------------------------------------
# Target
#---------------------------------------------------------------------------------------

list(APPEND ${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/delay-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/eeprom-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/gpio-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/rtc-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/spi-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/sx1276-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/uart-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/i2c-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/utilities.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hw_timer.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/samd21a/gcc/gcc/startup_samd21.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/samd21a/gcc/system_samd21.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hal/src/hal_atomic.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hal/src/hal_delay.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hal/src/hal_ext_irq.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hal/src/hal_gpio.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hal/src/hal_i2c_m_sync.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hal/src/hal_init.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hal/src/hal_io.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hal/src/hal_sleep.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hal/src/hal_spi_m_sync.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hal/src/hal_timer.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hal/utils/src/utils_assert.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hal/utils/src/utils_event.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hal/utils/src/utils_list.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hal/utils/src/utils_syscalls.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hpl/core/hpl_core_m0plus_base.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hpl/core/hpl_init.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hpl/dmac/hpl_dmac.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hpl/eic/hpl_eic.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hpl/gclk/hpl_gclk.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hpl/pm/hpl_pm.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hpl/rtc/hpl_rtc.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hpl/sercom/hpl_sercom.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hpl/sysctrl/hpl_sysctrl.c" 
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hpl/usart/usart_lite.c" 
)

add_library(${PROJECT_NAME} OBJECT EXCLUDE_FROM_ALL ${${PROJECT_NAME}_SOURCES})

target_compile_definitions(${PROJECT_NAME} PUBLIC -D__SAMD21G18A__)

# Add define if debbuger support is enabled
target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<BOOL:${USE_DEBUGGER}>:USE_DEBUGGER>)

# Add define if radio debug pins support is enabled
target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<BOOL:${USE_RADIO_DEBUG}>:USE_RADIO_DEBUG>)

# For debug builds set the symbol DEBUG
set(CMAKE_C_FLAGS_DEBUG -DDEBUG)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/cmsis
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/config
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hal
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hal/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hal/utils/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hpl/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hpl/dmac
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hpl/eic
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hpl/gclk
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hpl/pm
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hpl/port
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hpl/rtc
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hpl/sercom
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hpl/sysctrl
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hpl/usart 
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/hri
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/samd21a/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/samd21a/include/component
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/samd21a/include/instance
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samd21/samd21a/include/pio
    $<TARGET_PROPERTY:board,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:system,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:radio,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:peripherals,INTERFACE_INCLUDE_DIRECTORIES>
)

set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 11)
