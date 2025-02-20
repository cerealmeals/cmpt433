# app/CMakeLists.txt


# Include directories for the app
include_directories(include)
file(GLOB HAL_SOURCES "src/*.c")

# Create a library from HAL sources
add_library(hal_lib ${HAL_SOURCES})
