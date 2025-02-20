# app/CMakeLists.txt

file(GLOB MY_SOURCES "src/*.c")
add_executable(light_sampler ${MY_SOURCES})

# Include directories for the app
include_directories(include)
target_include_directories(light_sampler PRIVATE
    ${CMAKE_SOURCE_DIR}/hal/include
)

# Link against the HAL library
target_link_libraries(light_sampler PRIVATE hal_lib)

