# hal/CMakeLists.txt

# Include directories for the app
file(GLOB HAL_SOURCES ${CMAKE_SOURCE_DIR}/hal/src/*.c)

include_directories(include)


# Create a library from HAL sources
add_library(hal_lib ${HAL_SOURCES})