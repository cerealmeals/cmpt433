# Minimum version. Run on the host.
cmake_minimum_required(VERSION 3.18)
# Project info
project(
Assignment2
VERSION 1.0
DESCRIPTION "A game testing reactions using and led and I2C joystick"
LANGUAGES C
)




# Global Compiler options

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_COMPILER /usr/bin/aarch64-linux-gnu-gcc)
add_compile_options(-Wall -g  -Werror -Wpedantic -Wextra -fsanitize=address -pthread)
add_link_options(-fsanitize=address)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ~/cmpt433/public)

# Include subdirectories
# add_subdirectory(hal)
add_subdirectory(app)