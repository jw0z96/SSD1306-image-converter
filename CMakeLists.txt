cmake_minimum_required(VERSION 3.5)

project(oled-img-conv)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# TODO: set this for win/linux, or just include argp source
# set(ARGP_PATH /opt/homebrew/opt/argp-standalone)

add_executable(oledimgconv oledimgconv.c)

target_compile_options(oledimgconv PRIVATE -Wall -Werror)

# target_include_directories(oledimgconv PRIVATE ${ARGP_PATH}/include)
# target_link_libraries(oledimgconv PRIVATE ${ARGP_PATH}/lib/libargp.a)

# build an example for pico
# add_subdirectory(examples/pico-test)
