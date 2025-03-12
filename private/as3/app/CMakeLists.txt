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

find_package(ALSA REQUIRED)
target_link_libraries(Bop_It LINK_PRIVATE asound)

add_custom_command(TARGET Bop_It POST_BUILD
COMMAND "${CMAKE_COMMAND}" -E copy_directory
"${CMAKE_SOURCE_DIR}/wave-files"
"~/cmpt433/public/myApps/wave-files"
COMMENT "Copying WAVE files to public NFS directory")