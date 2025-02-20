# app/CMakeLists.txt

file(GLOB MY_SOURCES "src/*.c")
add_executable(Assignment2 ${MY_SOURCES})

# Include directories for the app
include_directories(include)
target_include_directories(Assignment2 PRIVATE
    ${CMAKE_SOURCE_DIR}/hal/include
)

# Link against the HAL library
target_link_libraries(Assignment2 PRIVATE hal_lib)

