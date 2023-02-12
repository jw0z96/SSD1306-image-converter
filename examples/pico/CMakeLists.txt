cmake_minimum_required(VERSION 3.5)

include(pico_sdk_import.cmake)

project(oled_image C CXX ASM)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(oled_image
	oled_image.c
	oled_utils.c
)

pico_add_extra_outputs(oled_image)

# pull in common dependencies and additional i2c hardware support
target_link_libraries(oled_image pico_stdlib hardware_i2c)
