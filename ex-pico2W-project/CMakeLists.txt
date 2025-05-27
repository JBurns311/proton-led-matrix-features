cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(example_pico2W)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
file(GLOB_RECURSE SOURCES "src/*.c" "src/*.cpp")
add_executable(example_pico2W
    ${SOURCES}
)

pico_enable_stdio_usb(example_pico2W 1)
pico_enable_stdio_uart(example_pico2W 0)

pico_add_extra_outputs(example_pico2W)

target_link_libraries(example_pico2W pico_stdlib)
