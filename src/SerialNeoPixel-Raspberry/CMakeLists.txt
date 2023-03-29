cmake_minimum_required(VERSION 3.13.4)
set(CMAKE_CXX_STANDARD 17)

#set(ENV{PICO_SDK_PATH} /mnt/c/Code/SDKs/pico/pico-sdk)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
#include(lib/pico_neopixels/pico_sdk_import.cmake)




project(SerialNeoPixel-Raspberry)
pico_sdk_init()

add_executable(SerialNeoPixel-Raspberry
        src/main.cpp
        include/Fade.h src/Fade.cpp
        include/LedController.h src/LedController.cpp
        include/RGB.h
        include/SerialController.h src/SerialController.cpp)


pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/src/ws2812byte.pio)

pico_add_extra_outputs(${PROJECT_NAME})

add_subdirectory(lib/pico_neopixels)
#add_subdirectory(src)
#add_subdirectory(include)

target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        hardware_pio
        pico_malloc
        pico_multicore
        pico_mem_ops
        pico_neopixel
        )

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

