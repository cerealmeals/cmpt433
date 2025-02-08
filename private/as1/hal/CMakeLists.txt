# hal/CMakeLists.txt

# Create a library for the HAL components
add_library(hal STATIC
    JoystickControl.c
    JoystickControl.h
    LEDcontrol.c
    LEDcontrol.h
    I2C.c
    I2C.h
)

# Include directories for HAL
target_include_directories(hal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Compiler options for HAL (if needed)
target_compile_options(hal PRIVATE -g -Wall -Werror -Wpedantic -Wextra)