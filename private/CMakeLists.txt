# Minimum version. Run on the host.
cmake_minimum_required(VERSION 3.18)
# Project info
project(
ReactionGame
VERSION 1.0
DESCRIPTION "A game testing reactions using and led and I2C joystick"
LANGUAGES C
)




# Compiler options
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ~/cmpt433/public)
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_COMPILER /usr/bin/aarch64-linux-gnu-gcc)
add_compile_options(-Wall -Werror -Wpedantic -Wextra)
add_executable( Reaction_Game
    ass1/app/main.c
    hal/JoystickControl.c
    hal/JoystickControl.h
    hal/LEDcontrol.h
    hal/LEDcontrol.c
)