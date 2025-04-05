# Minimum version. Run on the host.
cmake_minimum_required(VERSION 3.18)
# Project info
project(
finddot
VERSION 1.0
DESCRIPTION "Small game about finding a dot"
LANGUAGES C
)

# Find libgpiod
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiod)

# Include libgpiod headers
include_directories(${GPIOD_INCLUDE_DIRS})

# Global Compiler options
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_COMPILER /usr/bin/aarch64-linux-gnu-gcc)
add_compile_options(-Wall -g  -Werror -Wpedantic -Wextra)

# Enable PThread library for linking
add_compile_options(-pthread)
add_link_options(-pthread)

# Enable address sanitizer
# (Comment this out to make your code faster)
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ~/cmpt433/public/myApps)

# Include subdirectories
add_subdirectory(hal)
add_subdirectory(app)
add_subdirectory(lgpio)
add_subdirectory(lcd)

# Link libgpiod to the HAL library and the main executable
target_link_libraries(hal_lib PRIVATE ${GPIOD_LIBRARIES})
target_link_libraries(finddot PRIVATE hal_lib ${GPIOD_LIBRARIES})

