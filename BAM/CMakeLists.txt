cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(DMA)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
file(GLOB_RECURSE SOURCES "src/*.c" "src/*.cpp")
add_executable(DMA
    ${SOURCES}
)
include_directories(include)
target_include_directories(DMA PUBLIC include)

pico_enable_stdio_usb(DMA 1)
pico_enable_stdio_uart(DMA 0)

pico_add_extra_outputs(DMA)

target_link_libraries(DMA pico_stdlib hardware_pwm hardware_pio hardware_dma hardware_spi)
