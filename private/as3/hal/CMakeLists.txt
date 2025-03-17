# as3/hal/CMakeLists.txt

# Include directories for the app

file(GLOB HAL_SOURCES ${CMAKE_SOURCE_DIR}/hal/src/*.c)

include_directories(include)


# Create a library from HAL sources
add_library(hal_lib ${HAL_SOURCES})

# #need files in as3/lcd and as3/lgpio
target_link_libraries(hal_lib PRIVATE lcd lgpio)

