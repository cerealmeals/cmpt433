# app/CMakeLists.txt

file(GLOB MY_SOURCES "src/*.c")
add_executable(Bop_It ${MY_SOURCES})

# Include directories for the app
include_directories(include)
target_include_directories(Bop_It PRIVATE
    ${CMAKE_SOURCE_DIR}/hal/include
)

# Link against the HAL library
target_link_libraries(Bop_It PRIVATE hal_lib)
# target_link_libraries(Bop_It LINK_PRIVATE lcd)
# target_link_libraries(Bop_It LINK_PRIVATE lgpio)

