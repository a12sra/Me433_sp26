cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(pico_sdk_import.cmake)

project(dev_hid_composite C CXX ASM)

pico_sdk_init()

add_executable(dev_hid_composite
    main.c
    mpu6050.c
    usb_descriptors.c
)

target_include_directories(dev_hid_composite PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(dev_hid_composite
    pico_stdlib
    pico_unique_id
    tinyusb_device
    tinyusb_board
    hardware_i2c
    hardware_gpio
    m
)

pico_add_extra_outputs(dev_hid_composite)