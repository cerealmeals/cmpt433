# app/CMakeLists.txt

# Create an executable for the application
add_executable(Assignment2
    main.c
    network.c
    network.h
    shutdown.c
    shutdown.h
)

# Include directories for the app
target_include_directories(Assignment2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link the HAL library
# target_link_libraries(Assignment2 PRIVATE hal)