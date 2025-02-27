cmake_minimum_required(VERSION 3.13)
include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(bit-rhythm)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

pico_sdk_init()

add_executable(bit-rhythm 
    bit-rhythm.c
    ws2812.pio
)

pico_generate_pio_header(bit-rhythm ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

target_link_libraries(bit-rhythm
    pico_stdlib
    hardware_pwm
    hardware_pio
    hardware_gpio
    hardware_timer
)

pico_add_extra_outputs(bit-rhythm)
