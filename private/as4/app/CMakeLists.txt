# app/CMakeLists.txt

file(GLOB MY_SOURCES "src/*.c")
add_executable(finddot ${MY_SOURCES})

# Include directories for the app
include_directories(include)
target_include_directories(finddot PRIVATE
    ${CMAKE_SOURCE_DIR}/hal/include
)

# Link against the HAL library
target_link_libraries(finddot PRIVATE hal_lib)
target_link_libraries(finddot LINK_PRIVATE lcd)
target_link_libraries(finddot LINK_PRIVATE lgpio)

